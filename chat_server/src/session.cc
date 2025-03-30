#include <iostream>

#include <boost/asio/write.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>

#include "logic_system.hpp"
#include "server.hpp"
#include "session.hpp"

Session::Session(boost::asio::io_context &_ioc, Server *_server)
    : socket_(_ioc), server_(_server) {
  boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
  uuid_ = boost::uuids::to_string(a_uuid);
}

boost::asio::ip::tcp::socket &Session::get_socket() { return socket_; }
std::string const &Session::get_uuid() const { return uuid_; }

void Session::start() { async_read_head(TCP_MSG_HEAD_MEM_LENGTH); }

void Session::close() { socket_.close(); }

void Session::send(std::string const &_msg, short _msg_id) {
  send(_msg.c_str(), _msg.length(), _msg_id);
}

void Session::send(char const *_msg, int _msg_len, short _msg_id) {
  auto self = shared_from_this();
  std::lock_guard<std::mutex> gaurd(send_que_lock_);
  auto que_size = send_que_.size();
  // 队列满了，丢弃消息，限制内存用量
  if (que_size > TCP_MAX_SEND_QUE_SIZE) {
    std::cout << "Session: " << uuid_ << " send msg id: " << _msg_id
              << " failed, que size is " << que_size << std::endl;
    return;
  }

  send_que_.push(std::make_shared<TcpSendNode>(_msg, _msg_len, _msg_id));
  // 第一次放入消息队列时，挂起写事件，并由写事件回调继续挂起写事件处理队列剩余消息
  // que_size大于0说明已经有写事件准备启动或已经启动了
  if (que_size > 0) {
    return;
  }
  auto send_node = send_que_.front();
  boost::asio::async_write(
      socket_, boost::asio::buffer(send_node->data_, send_node->total_len_),
      [self](auto _ec, auto _bytes_transfered) {
        self->send_callback(_ec, _bytes_transfered);
      });
}
void Session::async_read_head(short _head_len) {
  auto self = shared_from_this();
  async_read_full(_head_len, [self, this,
                              _head_len](const boost::system::error_code &ec,
                                         std::size_t bytes_transfered) {
    try {
      if (ec) {
        std::cout << "handle read failed, error is " << ec.what() << std::endl;
        close();
        server_->remove_session(uuid_);
        return;
      }

      if (bytes_transfered < _head_len) {
        // 实际上不会进来，async_read_full的回调要么出错，要么读取到指定字节数
        std::cout << "read length not match, read [" << bytes_transfered
                  << "] , total [" << _head_len << "]" << std::endl;
        close();
        server_->remove_session(uuid_);
        return;
      }
      recv_head_node_->clear();
      memcpy(recv_head_node_->data_, data_, bytes_transfered);
          // 获取头部MSGID数据
          short msg_id = 0;
      memcpy(&msg_id, recv_head_node_->data_, TCP_MSG_ID_MEM_LENGTH);
      // 网络字节序转化为本地字节序
      msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
      std::cout << "msg_id is " << msg_id << std::endl;
      // id非法
      if (msg_id > TCP_MAX_ID_LENGTH) {
        std::cout << "invalid msg_id is " << msg_id << std::endl;
        server_->remove_session(uuid_);
        return;
      }
      short msg_len = 0;
      memcpy(&msg_len, recv_head_node_->data_ + TCP_MSG_ID_MEM_LENGTH, TCP_MSG_BODY_MEM_LENGTH);
      // 网络字节序转化为本地字节序
      msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
      std::cout << "msg_len is " << msg_len << std::endl;

      // 消息长度非法
      if (msg_len > TCP_MAX_MSG_LENGTH) {
        std::cout << "invalid data length is " << msg_len << std::endl;
        server_->remove_session(uuid_);
        return;
      }

      recv_msg_node_ = std::make_shared<TcpRecvNode>(msg_len, msg_id);
      async_read_body(msg_len);
    } catch (std::exception &e) {
      std::cout << "Exception code is " << e.what() << std::endl;
      close();
      server_->remove_session(uuid_);
    }
  });
}
void Session::async_read_body(short _body_len) {
  auto self = shared_from_this();
  async_read_full(_body_len, [self, this,
                              _body_len](const boost::system::error_code &ec,
                                         std::size_t bytes_transfered) {
    try {
      if (ec) {
        std::cout << "handle read failed, error is " << ec.what() << std::endl;
        close();
        server_->remove_session(uuid_);
        return;
      }

      if (bytes_transfered < _body_len) {
        std::cout << "read length not match, read [" << bytes_transfered
                  << "] , total [" << _body_len << "]" << std::endl;
        close();
        server_->remove_session(uuid_);
        return;
      }

      memcpy(recv_msg_node_->data_, data_, bytes_transfered);
      recv_msg_node_->cur_len_ += bytes_transfered;
      recv_msg_node_->data_[recv_msg_node_->total_len_] = '\0';
      std::cout << "receive data is " << recv_msg_node_->data_ << std::endl;
      // 此处将消息投递到逻辑队列中
      LogicSystem::get_instance()->post_msg_to_que(
          make_shared<LogicNode>(shared_from_this(), recv_msg_node_));
      // 继续监听头部接受事件
      async_read_head(TCP_MSG_HEAD_MEM_LENGTH);
    } catch (std::exception &e) {
        close();
        server_->remove_session(uuid_);
      std::cout << "Exception code is " << e.what() << std::endl;
    }
  });
}

// 读取完整长度
void Session::async_read_full(std::size_t maxLength,
                              async_read_callback_t handler) {
  ::memset(data_, 0, TCP_MAX_MSG_LENGTH);
  async_read_len(0, maxLength, handler);
}

// 封装asio的async_read_some函数，阻塞直到读取到指定字节数(类似async_read的功能)
void Session::async_read_len(std::size_t read_len, std::size_t total_len,
                             async_read_callback_t handler) {
  auto self = shared_from_this();
  socket_.async_read_some(
      // read_len是已读取长度
      boost::asio::buffer(data_ + read_len, total_len - read_len),
      [read_len, total_len, handler, self](const boost::system::error_code &ec,
                                           std::size_t bytesTransfered) {
        if (ec) {
          // 出现错误，调用回调函数
          handler(ec, read_len + bytesTransfered);
          return;
        }

        if (read_len + bytesTransfered >= total_len) {
          // 长度够了就调用回调函数
          handler(ec, read_len + bytesTransfered);
          return;
        }

        // 没有错误，且长度不足则继续读取
        self->async_read_len(read_len + bytesTransfered, total_len, handler);
      });
}

void Session::send_callback(boost::system::error_code &_ec,
                            std::size_t _bytes_transfered) {
  try {

    if (_ec) {
      std::cout << "Session: " << uuid_ << " send msg faild. error is "
                << _ec.what();
      close();
      server_->remove_session(uuid_);
      return;
    }

    auto self = shared_from_this();
    std::lock_guard<std::mutex> gaurd(send_que_lock_);
    send_que_.pop(); // 回调时说明asio已将本条消息发送结束，移出队列
    // 队列不空继续挂起写事件写数据给对端
    if (!send_que_.empty()) {
      auto &msg_to_send = send_que_.front();
      boost::asio::async_write(
          socket_,
          boost::asio::buffer(msg_to_send->data_, msg_to_send->total_len_),
          [self](auto _ec, auto _bytes_transfered) {
            self->send_callback(_ec, _bytes_transfered);
          });
    }
  } catch (std::exception const &e) {
    std::cout << "Exception occured when sending msg to peer. exception is "
              << e.what() << std::endl;
    close();
    server_->remove_session(uuid_);
  }
}

LogicNode::LogicNode(std::shared_ptr<Session> _session,
                     std::shared_ptr<TcpRecvNode> _recv_node)
    : session_(_session), recvnode_(_recv_node) {}