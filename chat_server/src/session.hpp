#ifndef SESSION_HPP
#define SESSION_HPP
#include <queue>

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>

#include "../../common/constant.hpp"
#include "../../common/tcp_msg_node.hpp"

class Server;
class Session : public std::enable_shared_from_this<Session> {
public:
  using async_read_callback_t =
      std::function<void(const boost::system::error_code &, std::size_t)>;
  Session(boost::asio::io_context &_ioc, Server *_server);
  boost::asio::ip::tcp::socket &get_socket();
  std::string const &get_uuid() const;
  void start();
  void close();
  void send(std::string const &_msg, short _msg_id);
  void send(char const *_msg, int _msg_len, short _msg_id);

private:
  void async_read_head(short _head_len);
  void async_read_body(short _body_len);
  void async_read_full(std::size_t maxLength, async_read_callback_t handler);

  void async_read_len(std::size_t read_len, std::size_t total_len,
                      async_read_callback_t handler);

  void send_callback(boost::system::error_code &_ec,
                     std::size_t _bytes_transfered);
  boost::asio::ip::tcp::socket socket_;
  std::string uuid_;
  Server *server_;
  std::shared_ptr<TcpMsgNode> recv_head_node_;
  std::shared_ptr<TcpRecvNode> recv_msg_node_;
  char data_[TCP_MAX_MSG_LENGTH];

  // 发送给对端消息的队列
  std::queue<std::shared_ptr<TcpSendNode>> send_que_;
  std::mutex send_que_lock_;
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