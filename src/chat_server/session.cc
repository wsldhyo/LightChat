#include "session.hpp"
#include "logic_system.hpp"
#include "manager/config_manager.hpp"
#include "manager/redis_manager.hpp"
#include "pool/iocontext_pool.hpp"
#include "server.hpp"
#include "utility/constant.hpp"
#include "utility/defer.hpp"
#include "utility/toolfunc.hpp"
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <json/writer.h>
Session::Session(tcp::socket peer, std::shared_ptr<Server> server)
    : peer_(std::move(peer)), server_(server),
      last_heartbeat_(std::chrono::steady_clock::now()) {
  session_id_ = generate_unique_string();
  recv_head_node_ = std::make_unique<TcpMsgNode>(TCP_MSG_HEAD_MEM_SIZE);
}

Session::~Session() {
  std::cout << "~session\n";
  close();
}

void Session::start() {
  async_read_len(recv_head_node_->data_, TCP_MSG_HEAD_MEM_SIZE, 0,
                 [self = shared_from_this()](boost::system::error_code ec,
                                             std::size_t bytes_transferred) {
                   self->readhead_callback(ec, bytes_transferred);
                 });
}

std::string const &Session::get_session_id() const { return session_id_; }

void Session::set_user_id(int32_t uid) { user_id_ = uid; }

int32_t Session::get_user_id() const { return user_id_; }

void Session::send(std::string const &msg, std::uint16_t msg_id) {
  this->send(msg.data(), msg.length(), msg_id);
}

void Session::send(char const *msg, std::size_t msg_len, std::uint16_t msg_id) {
  std::size_t send_que_size{0};
  {
    std::lock_guard<std::mutex> lock(send_que_mutex_);
    send_que_size = send_que_.size();
    if (send_que_size > TCP_MAX_SEND_QUE_SIZE) {
      // 超出消息队列容量，丢弃消息
      std::cout << "Queue overflow, and msg:<" << msg
                << "> will be discarded\n";
      return;
    }
    send_que_.push(std::make_unique<SendMsgNode>(msg, msg_len, msg_id));
    std::cout << "send body len:" << msg_len
              << " total len:" << send_que_.back()->length_ << '\n';
    std::cout << "send data is:" << msg << '\n';
    msg = send_que_.back()->data_;
  }

  // 队列有消息，说明后面的异步写操作已经启动或准备启动了
  if (send_que_size > 0) {
    return;
  }
  // 异步写
  boost::asio::async_write(
      peer_, boost::asio::buffer(msg, msg_len + TCP_MSG_HEAD_MEM_SIZE),
      [self = shared_from_this()](auto ec, auto bytes_transferred) {
        self->send_callback(ec, bytes_transferred);
      });
}

void Session::close() {
  bool expected = false;
  if (!closing_.compare_exchange_strong(expected, true)) {
    return; // already closing/closed，幂等退出
  }

  // 关闭 socket（以 non-throwing 形式）
  boost::system::error_code ec;
  peer_.shutdown(tcp::socket::shutdown_both, ec);
  peer_.close(ec);

  // TODO 将socket绑定的ioc归还iocontext池
}

void Session::readhead_callback(boost::system::error_code ec,
                                std::size_t bytes_transferred) {

  // 有错误发生，通常是对端关闭、本地网络错误等，需要关闭会话
  if (ec) {
    std::cout << "readhead error occurred, error is " << ec.message() << '\n';
    this->close();
    deal_exception_session();
    return;
  }
  try {
    if (auto srv = server_.lock()) {
      // 连接无效，可能被服务器踢人，关闭socket
      if (!srv->check_session_vaild(session_id_)) {
        close();
        return;
      }
    }
    update_heartbeat(); // 收到客户端数据包,说明连接存货，更新时间戳
    // 消息头读取完毕，读取消息体
    // 先从头部获取id和消息长度，并转为本地字节序
    std::uint16_t msg_id{0};
    memcpy(&msg_id, recv_head_node_->data_, TCP_MSG_ID_MEM_SIZE);
    msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
    if (msg_id > TCP_MAX_ID) {
      std::cout << "invaild msg id: " << msg_id << '\n';
      this->close(); // 客户端bug、网络乱流等导致的协议错误，需关闭会话
      return;
    }

    std::uint16_t msg_body_length{0};
    memcpy(&msg_body_length, recv_head_node_->data_ + TCP_MSG_ID_MEM_SIZE,
           TCP_MSG_LEN_MEM_SIZE);
    msg_body_length =
        boost::asio::detail::socket_ops::network_to_host_short(msg_body_length);

    if (msg_body_length > TCP_MAX_ID) {
      std::cout << "invalid msg length: " << msg_body_length << '\n';
      this->close(); // 长度非法，直接关
      return;
    }

    if (msg_body_length == 0) {
      std::cout << "msg length is zero!";
      return;
    }
    std::cout << "msg id is:" << msg_id << " msg len is:" << msg_body_length
              << '\n';

    // 构造接收缓冲区，开启异步读取消息体的回调函数
    recv_msg_node_ = std::make_unique<RecvMsgNode>(msg_body_length, msg_id);
    this->async_read_len(
        recv_msg_node_->data_, recv_msg_node_->length_, 0,
        [self = shared_from_this()](boost::system::error_code ec,
                                    std::size_t bytes_transferred) {
          self->readbody_callback(ec, bytes_transferred);
        });
  } catch (std::exception const &e) {
    std::cout << "logic exception in readhead callback: " << e.what() << '\n';
    // this->close();  TODO
    // deal_exception_session();
  }
}

void Session::readbody_callback(boost::system::error_code ec,
                                std::size_t bytes_transferred) {

  // 有错误发生，通常是对端关闭、本地网络错误等，需要关闭会话
  if (ec) {
    std::cout << "readhead error occurred, error is " << ec.message() << '\n';
    this->close();
    deal_exception_session();
    return;
  }
  try {
    // 连接无效，可能被服务器踢人，关闭socket
    if (auto srv = server_.lock()) {
      if (!srv->check_session_vaild(session_id_)) {
        close();
        return;
      }
    }
    update_heartbeat(); // 收到客户端数据包,说明连接存货，更新时间戳
    //  消息读取完毕，将其投递给LogicSystem的队列
    LogicSystem::get_instance()->post_msg(std::move(recv_msg_node_),
                                          shared_from_this());
    // 继续尝试读取
    async_read_len(recv_head_node_->data_, TCP_MSG_HEAD_MEM_SIZE, 0,
                   [self = shared_from_this()](boost::system::error_code ec,
                                               std::size_t bytes_transferred) {
                     self->readhead_callback(ec, bytes_transferred);
                   });
  } catch (std::exception const &e) {
    std::cout << "logic exception in readbody callback: " << e.what() << '\n';
    // this->close();  TODO
    // deal_exception_session();
  }
}

void Session::send_callback(boost::system::error_code ec,
                            std::size_t bytes_transferred) {

  if (ec) {
    std::cout << "send error occurred. error is " << ec.message() << '\n';
    this->close();
    deal_exception_session();
    return;
  }

  try {
    std::size_t send_que_size{0};
    {
      // 消息发送完成，弹出仍在队列中的消息体
      std::lock_guard<std::mutex> lock(send_que_mutex_);
      send_que_.pop();
      send_que_size = send_que_.size();
    }

    // 队列还有消息，继续发送位于队头的消息
    if (send_que_size > 0) {
      // 继续发送
      char const *msg{nullptr};
      std::size_t msg_len{0};
      {
        std::lock_guard<std::mutex> lock(send_que_mutex_);
        msg = send_que_.front()->data_;
        msg_len = send_que_.front()->length_;
      }
      boost::asio::async_write(
          peer_, boost::asio::buffer(msg, msg_len),
          [self = shared_from_this()](auto ec, auto bytes_transferred) {
            self->send_callback(ec, bytes_transferred);
          });
    }
  } catch (std::exception const &e) {
    std::cout << "logic exception in send callback: " << e.what() << '\n';
    // this->close();  TODO
    // deal_exception_session();
  }
}

void Session::deal_exception_session() {

  //加锁清除session
  auto uid_str = std::to_string(user_id_);
  auto lock_key = std::string(REDIS_LOCK_PREFIX) + uid_str;
  // 尝试获取分布式锁
  auto identifier = RedisMgr::get_instance()->acquire_lock(
      lock_key, REDIS_LOCK_TIMEOUT, REDIS_ACQUIRE_TIMEOUT);
  // 调用结束后，移除session，并释放分布式锁
  Defer defer([identifier, lock_key, self = shared_from_this(), this]() {
    if (auto srv = server_.lock()) {
      srv->remove_session(session_id_);
    }
    RedisMgr::get_instance()->release_lock(lock_key, identifier);
  });

  if (identifier.empty()) {
    // 获取锁失败则直接返回，不能删除Redis中的相关信息
    return;
  }
  // 从 Redis 获取当前用户绑定的 session_id
  std::string redis_session_id = "";
  auto session_key = std::string(REDIS_USER_SESSION_PREFIX) + uid_str;
  auto bsuccess = RedisMgr::get_instance()->get(session_key, redis_session_id);
  if (!bsuccess) {
    return;
  }

  if (redis_session_id != session_id_) {
    // 如果 Redis 中 session_id 与当前不一致，说明用户在其他服务器登录，
    // 直接返回，不能删除Redis中的相关信息
    return;
  }

  RedisMgr::get_instance()->del(session_key);
  //清除用户登录信息
  RedisMgr::get_instance()->del(std::string(REDIS_USER_BASE_INFO_PREFIX) +
                                uid_str);
}

void Session::notify_offline(int32_t uid) {
  Json::Value rtvalue;
  rtvalue["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);
  rtvalue["uid"] = uid;

  Json::StreamWriterBuilder writer;
  writer["indentation"] = ""; // 紧凑格式
  std::string return_str = Json::writeString(writer, rtvalue);
  // 通知客户端退出登录界面
  send(return_str, static_cast<uint16_t>(ReqId::ID_NOTIFY_OFFLINE_REQ));
  return;
}
bool Session::is_heartbeat_expired(
    std::chrono::steady_clock::time_point const &now) {
  auto diff_sec = std::chrono::duration_cast<std::chrono::seconds>(
      now - last_heartbeat_.load(std::memory_order_relaxed));
  if (diff_sec > HEARTBEAT_THRESHOLD) {
    std::cout << "Heartbeat expired diff_sec:" << diff_sec.count()
              << " session id is " << session_id_ << '\n';
    return true;
  }
  return false;
}

void Session::update_heartbeat() {
  last_heartbeat_.store(std::chrono::steady_clock::now());
}
