#include "logic_system.hpp"
#include "manager/config_manager.hpp"
#include "manager/mysql_manager.hpp"
#include "manager/redis_manager.hpp"
#include "rpc_client/status_rpc_client.hpp"
#include "session.hpp"
#include "user_manager.hpp"
#include "utility/constant.hpp"
#include "utility/defer.hpp"
#include "utility/toolfunc.hpp"
#include "utility/userinfo.hpp"
#include <iostream>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
LogicSystem::LogicSystem() : b_stop_(false) {
  register_msg_handler();
  work_thread_ = std::thread(&LogicSystem::deal_msg, this);
}

LogicSystem::~LogicSystem() {
  b_stop_ = true;
  deal_msg_cond_.notify_one();
  if (work_thread_.joinable()) {
    work_thread_.join();
  }
}

void LogicSystem::post_msg(std::unique_ptr<RecvMsgNode> msg,
                           std::shared_ptr<Session> session) {
  {
    std::lock_guard<std::mutex> lock(msg_que_mutex_);
    auto que_size{recv_msg_que_.size()};
    if (que_size > TCP_MAX_LOGIG_QUE_SIZE) {
      std::cout << "Logic queue overflow. msg;[" << msg->data_
                << "] will be discarded\n";
      return;
    }
    recv_msg_que_.emplace(std::move(msg), session);
    // 队列为空时插入，则通知后台线程开始处理
    if (que_size == 0) {
      deal_msg_cond_.notify_one();
    }
  }
}

void LogicSystem::deal_msg() {
  decltype(recv_msg_que_) batch_msg;
  LogicNode logic_node;
  while (!b_stop_) {
    {
      {
        std::unique_lock<std::mutex> lock(msg_que_mutex_);
        while (recv_msg_que_.empty() && !b_stop_) {
          deal_msg_cond_.wait_for(lock, std::chrono::milliseconds{500});
        }
        batch_msg.swap(recv_msg_que_);
      }

      while (!batch_msg.empty()) {
        logic_node = std::move(batch_msg.front());
        batch_msg.pop();
        std::cout << "recv_msg id  is " << logic_node.msg_->msg_id_
                  << std::endl;
        auto call_back_iter =
            handlers_.find(static_cast<ReqId>(logic_node.msg_->msg_id_));
        if (call_back_iter == handlers_.end()) {
          std::cout << "msg id [" << logic_node.msg_->msg_id_
                    << "] handler not found" << std::endl;
          continue;
        }

        call_back_iter->second(
            logic_node.session_, static_cast<ReqId>(logic_node.msg_->msg_id_),
            std::string_view(logic_node.msg_->data_, logic_node.msg_->length_));
      }
    }
  }
}

void LogicSystem::register_msg_handler() {
  // TODO 这里没必要再请求一次StatusServer进行验证，
  //利用JWT验证流程，用同一个Secret验证id和token

  // 注册登录聊天服务器请求
  handlers_.emplace(
      ReqId::ID_CHAT_LOGIN, std::move([this](std::shared_ptr<Session> session,
                                             ReqId id, std::string_view msg) {
        Json::Reader reader;
        Json::Value root;
        reader.parse(msg.data(), msg.data() + msg.size(), root);
        auto uid = root["uid"].asInt();
        std::cout << "user login uid is  " << uid << " user token  is "
                  << root["token"].asString() << std::endl;

        //从状态服务器获取token, 验证token是否准确
        auto rsp = StatusGrpcClient::getinstance()->login(
            uid, root["token"].asString());
        Json::Value rtvalue; //要返回的数据
        Defer defer([this, &rtvalue, session]() {
          std::string return_str = rtvalue.toStyledString();
          session->send(return_str,
                        static_cast<uint16_t>(ReqId::ID_CHAT_LOGIN_RSP));
        });

        rtvalue["error"] = rsp.error();
        if (rsp.error() != static_cast<int32_t>(ErrorCodes::NO_ERROR)) {
          return;
        }
        auto uid_str = std::to_string(uid);
        UserInfo user_info;
        std::string base_key =
            std::string(REDIS_USER_BASE_INFO_PREFIX) + uid_str;
        auto res = get_base_info(base_key, uid, &user_info);
        if (!res) {
          rtvalue["error"] = static_cast<int32_t>(ErrorCodes::UID_INVALID);
          return;
        }

        rtvalue["uid"] = uid;
        rtvalue["pwd"] = user_info.pwd;
        rtvalue["name"] = user_info.name;
        rtvalue["email"] = user_info.email;
        rtvalue["nick"] = user_info.nick;
        rtvalue["desc"] = user_info.desc;
        rtvalue["sex"] = user_info.sex;
        rtvalue["icon"] = user_info.icon;

        // TODO 从数据库获取申请列表

        // 获取好友列表
        auto cfg = ConfigManager::getinstance();
        auto server_name = cfg->get_value("SelfServer", "name");
        auto login_count = RedisMgr::getinstance()->h_get(
            REDIS_LOGIN_COUNT_PREFIX, server_name);
        int count = 0;
        if (!login_count.empty()) {
          auto conv_res = string_to_int(login_count, count);
          if (conv_res != ErrorCodes::NO_ERROR) {
            std::cout << "login count val error:" << login_count << '\n';
          }
        }
        ++count;
        RedisMgr::getinstance()->h_set(REDIS_LOGIN_COUNT_PREFIX, server_name,
                                       std::to_string(count));
        // session和user绑定，一个用户的会话由一个session维护
        session->set_user_id(uid);
        std::string ip_key = std::string(REDIS_USER_IP_PREFIX) + uid_str;
        // 为用户设置登录ip对应server的名字
        RedisMgr::getinstance()->set(ip_key, server_name);
        // UserManager中将uid和session绑定，方便后续服务器踢人操作
        UserManager::getinstance()->set_user_session(uid, session);
        return;
      }));
}

bool LogicSystem::get_base_info(std::string base_key, int uid,
                                UserInfo *userinfo) {
  // 先到Redis中查询数据
  std::string info_str = "";
  bool b_base = RedisMgr::getinstance()->get(base_key, info_str);
  if (b_base) {
    Json::Reader reader;
    Json::Value root;
    reader.parse(info_str, root);
    userinfo->uid = root["uid"].asInt();
    userinfo->name = root["name"].asString();
    userinfo->pwd = root["pwd"].asString();
    userinfo->email = root["email"].asString();
    userinfo->nick = root["nick"].asString();
    userinfo->desc = root["desc"].asString();
    userinfo->sex = root["sex"].asInt();
    userinfo->icon = root["icon"].asString();
    std::cout << "user login uid is  " << userinfo->uid << " name  is "
              << userinfo->name << " pwd is " << userinfo->pwd << " email is "
              << userinfo->email << '\n';
  } else {
    // Redis中没有，就去数据库查找
    auto find_res = MysqlMgr::getinstance()->get_user(uid);
    if (find_res == std::nullopt) {
      return false;
    }

    *userinfo = std::move(find_res.value());

    Json::Value redis_root;
    redis_root["uid"] = uid;
    redis_root["pwd"] = userinfo->pwd;
    redis_root["name"] = userinfo->name;
    redis_root["email"] = userinfo->email;
    redis_root["nick"] = userinfo->nick;
    redis_root["desc"] = userinfo->desc;
    redis_root["sex"] = userinfo->sex;
    redis_root["icon"] = userinfo->icon;
    //将数据库中的数据添加到Redis中，方便下次查找
    RedisMgr::getinstance()->set(base_key, redis_root.toStyledString());
  }

  return true;
}