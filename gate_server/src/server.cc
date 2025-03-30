#include "server.hpp"
#include "http_connection.hpp"
#include "io_context_pool.hpp"
#include <boost/beast/core/error.hpp>
#include <exception>
#include <iostream>
#include <memory>
Server::Server(asio::io_context &_ioc, unsigned short _port)
    : acceptor_(_ioc, tcp::endpoint(tcp::v4(), _port)), ioc_(_ioc) {}

void Server::start() {
  if (b_stop_.load(std::memory_order_acquire)) {
    return;
  }

  std::cout << "start accept connection\n";
  auto &ioc = IOContextPool::get_instance()->get_io_context();
  auto new_connection = std::make_shared<HttpConnection>(ioc);
  auto self = shared_from_this();
  acceptor_.async_accept(
      new_connection->get_socket(),
      [new_connection, self](beast::error_code _ec) {
        try {
          // 出错，关闭当前链接，挂起监听事件准备监听其他连接
          if (_ec) {
            self->start();
            return;
          }
          std::cout << "accept: "
                    << new_connection->get_socket()
                           .remote_endpoint()
                           .address()
                           .to_string()
                    << std::endl;
          new_connection->start(); // 开始异步读取对端传来的HTTP数据
          self->start(); // 异步挂起另一个监听连接，处理另一个连接请求

        } catch (std::exception const &e) {
          std::cout << "listen exception occur, error is " << e.what();
        }
      });
}