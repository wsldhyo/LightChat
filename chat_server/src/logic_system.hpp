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

class LogicNode;
class Session;
class LogicSystem : public Singleton<LogicSystem> {
public:
  using msg_id_t = short;
  using callback_t = std::function<void(std::shared_ptr<Session>, msg_id_t, std::string const&)>;
  ~LogicSystem();

  void post_msg_to_que(std::shared_ptr<LogicNode> _logic_node);
private:
    // 友元使得单例基类可以访问构造
    friend class Singleton<LogicSystem>;
    LogicSystem();

    std::map<msg_id_t, callback_t> callbacks_;
};
#endif