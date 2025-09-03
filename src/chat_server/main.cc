#include "manager/config_manager.hpp"
#include "pool/iocontext_pool.hpp"
#include "server.hpp"
#include "utility/toolfunc.hpp"
#include <boost/asio/signal_set.hpp>
#include <iostream>
#include <string>
int main(int argc, char *argv[]) {
  int port{0};

  {
    auto cfg = ConfigManager::getinstance();
    cfg->parse();
    auto res = string_to_int((*cfg)["SelfServer"]["port"], port);
    if (res != ErrorCodes::NO_ERROR || port <= 0 && port > 65535) {
      std::cout << "Get port error: port is " << port << '\n';
      return 1;
    }
  }
  try {
    auto pool = IoContextPool::getinstance();
    boost::asio::io_context io_context;
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&io_context, pool](auto, auto) {
      io_context.stop();
      pool->stop();
    });
    auto s = std::make_shared<Server>(io_context, port);
    s->start_accept();
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "main func: exception occurred: " << e.what() << std::endl;
  }

  return 0;
}