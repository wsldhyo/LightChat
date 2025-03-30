#include <iostream>

#include <boost/asio/signal_set.hpp>
#include "../../common/io_context_pool.hpp"
#include "../../common/config_manager.hpp"
#include "server.hpp"
int main(int argc, char *argv[]) {
  try {
    auto sp_confid_mgr = ConfigManager::get_instance();
    auto &cfg = *sp_confid_mgr;
    cfg.parse();
    auto pool = IOContextPool::get_instance();
    boost::asio::io_context io_context;
    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&io_context, pool](auto, auto) {
      io_context.stop();
      pool->stop();
    });
    auto port_str = cfg["SelfServer"]["port"];
    Server s(io_context, atoi(port_str.c_str()));
    io_context.run();
  } catch (std::exception &e) {
    std::cerr << "Exception: " << e.what() << std::endl;
  }
  return 0;
}