#ifndef SESSION_HPP
#define SESSION_HPP
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "../../common/tcp_msg_node.hpp"
#include "../../common/constant.hpp"

class Server;
class Session : public std::enable_shared_from_this<Session> {
public:
  Session(boost::asio::io_context &_ioc, Server *_server);
  boost::asio::ip::tcp::socket  &get_socket();
  std::string const &get_uuid() const;
  void start();
  void close();

private:
  void async_read_head(short _head_len);
  void async_read_full(
      std::size_t maxLength,
      std::function<void(const boost::system::error_code &, std::size_t)>
          handler);

  void async_read_len(
      std::size_t read_len, std::size_t total_len,
      std::function<void(const boost::system::error_code &, std::size_t)>
          handler);

  boost::asio::ip::tcp::socket socket_;
  std::string uuid_;
  Server *server_;
  std::shared_ptr<TcpMsgNode> recv_head_node_;
  std::shared_ptr<TcpRecvNode> recv_msg_node_;
  std::shared_ptr<TcpMsgNode> send_msg_node_;
  char data_[TCP_MAX_MSG_LENGTH];
};
class LogicNode {
  friend class LogicSystem;

public:
  LogicNode(std::shared_ptr<Session>, std::shared_ptr<TcpRecvNode>);

private:
  std::shared_ptr<Session> session_;
  std::shared_ptr<TcpRecvNode> recvnode_;
};
#endif