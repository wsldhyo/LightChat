#ifndef GATE_SERVER_HPP
#define GATE_SERVER_HPP

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <memory>
namespace asio = boost::asio;
using tcp = asio::ip::tcp;
class HttpConnection;
class Server : public std::enable_shared_from_this<Server> {
public:
  Server(asio::io_context &_ioc, unsigned short _port);
  void start();

private:
  tcp::acceptor acceptor_;
  asio::io_context &ioc_;
  std::atomic_bool b_stop_;
};

#endif