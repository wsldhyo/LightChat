#ifndef LOGIC_SYSTEM_HPP
#define LOGIC_SYSTEM_HPP

#include "../../common/singleton.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <functional>
#include <map>
#include <memory>
#include <string>
class HttpConnection;
using http_handler_t = std::function<void(std::shared_ptr<HttpConnection>)>;
class LogicSystem : public Singleton<LogicSystem> {
public:
  ~LogicSystem();
  bool handle_get_request(std::string, std::shared_ptr<HttpConnection>);
  bool handle_post_request(std::string, std::shared_ptr<HttpConnection>);
  void register_get_handler(std::string, http_handler_t handler);
  void register_post_handler(std::string, http_handler_t handler);

private:
    // 友元使得单例基类可以访问构造
    friend class Singleton<LogicSystem>;
    LogicSystem();

    std::map<std::string, http_handler_t> post_handlers_;
    std::map<std::string, http_handler_t> get_hanlders_;
};
#endif