#include "server.hpp"
#include <boost/asio/signal_set.hpp>
#include <boost/beast/core/error.hpp>
#include <iostream>
int main(int argc, char const *argv[]) {
  try {
    auto port = static_cast<unsigned short>(8080);
    asio::io_context ioc{1};
    asio::signal_set sigals(ioc, SIGINT, SIGTERM);
    sigals.async_wait([&ioc](beast::error_code _ec, int _sigal_num) {
      if (_ec) {
        return;
      }
      ioc.stop();
    });
    std::make_shared<Server>(ioc, port)->start();
    ioc.run();
  } catch (std::exception const &e) {
    std::cout << "Serve Error: " << e.what() << std::endl;
  }
  return 0;
}