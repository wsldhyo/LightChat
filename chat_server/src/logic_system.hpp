#ifndef LOGIC_SYSTEM_HPP
#define LOGIC_SYSTEM_HPP

#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <condition_variable>
#include <thread>
#include "../../common/singleton.hpp"
#include "../../common/constant.hpp"

class LogicNode;
class Session;
class LogicSystem : public Singleton<LogicSystem> {
public:
  using callback_t = std::function<void(std::shared_ptr<Session>, RequestID, std::string const&)>;
  ~LogicSystem();

  void post_msg_to_que(std::shared_ptr<LogicNode> _logic_node);
private:
    // 友元使得单例基类可以访问构造
    friend class Singleton<LogicSystem>;
    LogicSystem();
    void regiser_callback();
    void deal_message();
    std::map<RequestID, callback_t> callbacks_;
    std::queue<std::shared_ptr<LogicNode>> msg_queue_;
    std::mutex msg_que_lock_;
    std::condition_variable cond_;
    bool b_stop_;
    std::thread work_thread_;
};
#endif