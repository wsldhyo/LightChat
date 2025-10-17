#include "manager/config_manager.hpp"
#include "server.hpp"
#include "utility/toolfunc.hpp"
#include <exception>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
using port_t = unsigned short;
int main(int argc, char *argv[]) {
  port_t port{0};
  {

    auto config_mgr = ConfigManager::get_instance();
    config_mgr->parse("basic_config.ini"sv);
    config_mgr->print();
    string_to_int((*config_mgr)["GateServer"]["port"], port);
  }

  try {
    asio::io_context ioc{1};
    asio::signal_set signals(ioc, SIGINT, SIGTERM);
    signals.async_wait([&ioc](boost::system::error_code ec, int sig_number) {
      if (ec) {
        return;
      }
      ioc.stop();
    });
    std::make_shared<Server>(ioc, port)->start();
    ioc.run();
  } catch (std::exception &e) {
    std::cout << "gate server main exception occured: " << e.what() << '\n';
    return 1;
  }

  return 0;
}