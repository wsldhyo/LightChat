#include "config_manager.hpp"
#include "server.hpp"
#include "toolfunc.hpp"
#include <boost/asio/signal_set.hpp>
#include <boost/beast/core/error.hpp>
#include <charconv>
#include <iostream>
int main(int argc, char const *argv[]) {
  auto& cfg_mgr = *ConfigManager::get_instance();
  auto error_code = cfg_mgr.parse();
  if (error_code != ErrorCode::NO_ERROR) {
    return static_cast<int>(error_code);
  }
  unsigned short gate_port;
  auto const &gate_port_str = cfg_mgr["GateServer"]["port"];
  auto trans_res = std::from_chars(
      gate_port_str.c_str(), gate_port_str.c_str() + gate_port_str.length(),
      gate_port);
  if (trans_res.ec != std::errc()) {
    std::cout << "gate port error" << std::endl;
    return static_cast<int>(ErrorCode::PARSE_GATE_PORT_ERROR);
  }

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