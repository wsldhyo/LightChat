#include <iostream>

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

void Session::start() { async_read_head(TCP_MSG_HEAD_LENGTH); }

void Session::close() { socket_.close(); }

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

      // 走到这里，一定是读取到头部长度的字节, 因此直接处理头部信息
      memcpy(recv_msg_node_->data_, data_, bytes_transfered);
      recv_msg_node_->cur_len_ += bytes_transfered;
      recv_msg_node_->data_[recv_msg_node_->total_len_] = '\0';
      std::cout << "receive data is " << recv_msg_node_->data_ << std::endl;
      // 此处将消息投递到逻辑队列中
      LogicSystem::get_instance()->post_msg_to_que(
          make_shared<LogicNode>(shared_from_this(), recv_msg_node_));
      // 继续监听头部接受事件
      async_read_head(TCP_MSG_HEAD_LENGTH);
    } catch (std::exception &e) {
      std::cout << "Exception code is " << e.what() << std::endl;
      close();
      server_->remove_session(uuid_);
    }
  });
}
// 读取完整长度
void Session::async_read_full(
    std::size_t maxLength,
    std::function<void(const boost::system::error_code &, std::size_t)>
        handler) {
  ::memset(data_, 0, TCP_MAX_MSG_LENGTH);
  async_read_len(0, maxLength, handler);
}

// 封装asio的async_read_some函数，阻塞直到读取到指定字节数(类似async_read的功能)
void Session::async_read_len(
    std::size_t read_len, std::size_t total_len,
    std::function<void(const boost::system::error_code &, std::size_t)>
        handler) {
  auto self = shared_from_this();
  socket_.async_read_some(
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
LogicNode::LogicNode(std::shared_ptr<Session> _session,
                     std::shared_ptr<TcpRecvNode> _recv_node)
    : session_(_session), recvnode_(_recv_node) {}