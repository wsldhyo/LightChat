#include "server.hpp"
#include "httpconn.hpp"
#include "pool/iocontext_pool.hpp"
#include <iostream>

Server::Server(boost::asio::io_context &ioc, unsigned short &port)
    : ioc_(ioc), acceptor_(ioc, tcp::endpoint(tcp::v4(), port)) {
  std::cout << "Server listen port:" << port << '\n';
}

void Server::start() {
  auto self = shared_from_this();
  auto &io_context = IoContextPool::get_instance()->get_iocontext();
  std::shared_ptr<HttpConn> new_con = std::make_shared<HttpConn>(io_context);
  acceptor_.async_accept(
      new_con->get_socket(), [self, new_con](beast::error_code ec) {
        try {
          //出错则放弃这个连接，继续监听新连接
          if (ec) {
            self->start();
            return;
          }
          //处理新链接，创建HpptConnection类管理新连接
          std::cout << "new connection from:"
                    << new_con->get_socket().remote_endpoint() << '\n';
          new_con->start();
          //继续监听其他连接请求
          self->start();
        } catch (std::exception &exp) {
          std::cout << "exception is " << exp.what() << '\n';
          self->start();
        }
      });
}