#include "server.hpp"
#include "http_connection.hpp"
#include <boost/beast/core/error.hpp>
#include <exception>
#include <iostream>
#include <memory>

Server::Server(asio::io_context &_ioc, unsigned short _port)
    : acceptor_(_ioc, tcp::endpoint(tcp::v4(), _port)), ioc_(_ioc),
      listen_socket_(_ioc) {}

void Server::start() {
  std::cout<< "start accept connection\n";
  auto self = shared_from_this();
  acceptor_.async_accept(listen_socket_, [self](beast::error_code _ec) {
    try {
      // 出错，关闭当前链接，挂起监听事件准备监听其他连接
      if (_ec) {
        self->start();
        return;
      }
      std::cout<< "accept: " << self->listen_socket_.remote_endpoint().address().to_string() << std::endl;
      // 没有错误， 挂起新的监听事件。监听其他连接
      // 创建新连接，并且让新创建的HttpConnection管理listen_socket_
      std::make_shared<HttpConnection>(std::move(self->listen_socket_))->start();
      self->start();
      
    } catch (std::exception const &e) {
      std::cout << "listen exception occur, error is " << e.what();
    }
  });
}