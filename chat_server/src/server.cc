#include "server.hpp"
#include "../../common/io_context_pool.hpp"
#include "session.hpp"
#include <iostream>

Server::Server(boost::asio::io_context &_ioc, int _port)
    : ioc_(_ioc), port_(_port),
      acceptor_(_ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(),
                                                     _port)) {

  start_accept();
}

void Server::start_accept() {
  auto &ioc = IOContextPool::get_instance()->get_io_context();
  auto new_session = std::make_shared<Session>(ioc, this);
  acceptor_.async_accept(new_session->get_socket(), [new_session,
                                                     this](auto _ec) {
    if (_ec) {
      {
        std::lock_guard<std::mutex> gaurd(mutex_);
        sessions_.insert(std::make_pair(new_session->get_uuid(), new_session));
        std::cout << "Accept peer: " << new_session->get_socket().remote_endpoint() << std::endl;
      }
      new_session->start();
    } else {
        std::cerr << "Server accept error: " << _ec.what() << std::endl; 
    }
    start_accept();     // 继续监听下一个连接请求
  });
}

void Server::remove_session(std::string const& _uuid){
    std::lock_guard<std::mutex> gaurd(mutex_);
    sessions_.erase(_uuid);
}