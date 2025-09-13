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
#include <json/writer.h>

/**
 * @brief 将UserInfo转换为Json::Value
 *
 * @param user 待转换的UserInfo
 * @return Json::Value
 */
Json::Value userinfo_to_json(const UserInfo &user) {
  Json::Value root;
  root["uid"] = user.uid;
  root["name"] = user.name;
  root["pwd"] = user.pwd;
  root["email"] = user.email;
  root["nick"] = user.nick;
  root["desc"] = user.desc;
  root["sex"] = user.sex;
  root["icon"] = user.icon;
  return root;
}

/**
 * @brief 将Json::Value转换为UserInfo
 * @param root
 * @return UserInfo
 */
UserInfo json_to_userinfo(const Json::Value &root) {
  UserInfo user;
  user.uid = root["uid"].asInt();
  user.name = root["name"].asString();
  user.pwd = root["pwd"].asString();
  user.email = root["email"].asString();
  user.nick = root["nick"].asString();
  user.desc = root["desc"].asString();
  user.sex = root["sex"].asInt();
  user.icon = root["icon"].asString();
  return user;
}

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
  // 注册登录聊天服务器请求
  handlers_.emplace(ReqId::ID_CHAT_LOGIN_REQ, [this](
                                              std::shared_ptr<Session> session,
                                              ReqId id, std::string_view msg) {
    Json::CharReaderBuilder reader_builder;
    std::unique_ptr<Json::CharReader> reader(reader_builder.newCharReader());
    Json::Value root;
    if (!reader->parse(msg.data(), msg.data() + msg.size(), &root, nullptr)) {
      Json::Value rsp_json;
      rsp_json["error"] = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
      session->send(rsp_json.toStyledString(),
                    static_cast<uint16_t>(ReqId::ID_CHAT_LOGIN_RSP));
      return;
    }

    int uid = root["uid"].asInt();
    std::string token = root["token"].asString();
    std::cout << "user login uid: " << uid << ", token: " << token << std::endl;

    Json::Value rtvalue;

    // 从状态服务器验证 token
    auto rsp = StatusGrpcClient::getinstance()->login(uid, token);
    if (rsp.error() != static_cast<int32_t>(ErrorCodes::NO_ERROR)) {
      rtvalue["error"] = rsp.error();
      session->send(rtvalue.toStyledString(),
                    static_cast<uint16_t>(ReqId::ID_CHAT_LOGIN_RSP));
      return;
    }

    // 获取用户基本信息
    UserInfo user_info;
    std::string base_key =
        std::string(REDIS_USER_BASE_INFO_PREFIX) + std::to_string(uid);
    if (!get_base_info(base_key, uid, user_info)) {
      rtvalue["error"] = static_cast<int32_t>(ErrorCodes::UID_INVALID);
      session->send(rtvalue.toStyledString(),
                    static_cast<uint16_t>(ReqId::ID_CHAT_LOGIN_RSP));
      return;
    }

    // 成功返回用户信息 + error=0
    rtvalue = userinfo_to_json(user_info);
    rtvalue["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);

    // 更新登录计数
    auto cfg = ConfigManager::getinstance();
    auto server_name = cfg->get_value("SelfServer", "name");
    auto login_count =
        RedisMgr::getinstance()->h_get(REDIS_LOGIN_COUNT_PREFIX, server_name);
    int count = 0;
    if (!login_count.empty()) {
      string_to_int(login_count, count);
    }
    ++count;
    RedisMgr::getinstance()->h_set(REDIS_LOGIN_COUNT_PREFIX, server_name,
                                   std::to_string(count));

    // session绑定uid，缓存登录IP和session
    session->set_user_id(uid);
    std::string ip_key =
        std::string(REDIS_USER_IP_PREFIX) + std::to_string(uid);
    RedisMgr::getinstance()->set(ip_key, server_name);
    UserManager::getinstance()->set_user_session(uid, session);

    // 发送回包
    session->send(rtvalue.toStyledString(),
                  static_cast<uint16_t>(ReqId::ID_CHAT_LOGIN_RSP));
  });

  // 处理客户端搜索用户请求
  handlers_.emplace(
      ReqId::ID_SEARCH_USER_REQ,
      [this](std::shared_ptr<Session> session, ReqId id, std::string_view msg) {
        Json::CharReaderBuilder reader_builder;
        std::unique_ptr<Json::CharReader> reader(
            reader_builder.newCharReader());
        Json::Value root;
        Json::Value ret_value;

        if (!reader->parse(msg.data(), msg.data() + msg.size(), &root,
                           nullptr)) {
          ret_value["error"] =
              static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
          session->send(ret_value.toStyledString(),
                        static_cast<uint16_t>(ReqId::ID_SEARCH_USER_RSP));
          return;
        }

        std::string uid_str = root["uid"].asString();
        std::cout << "User Searchinfo uid: " << uid_str << '\n';

        int32_t uid = 0;
        auto ec = string_to_int(uid_str, uid);
        if (ec == ErrorCodes::NO_ERROR) {
          get_user_by_uid(uid_str, uid, ret_value);
        } else {
          get_user_by_name(uid_str, ret_value);
        }

        // 返回结果（ret_value里已经带 error 字段）
        session->send(ret_value.toStyledString(),
                      static_cast<uint16_t>(ReqId::ID_SEARCH_USER_RSP));
      });
}

bool LogicSystem::get_base_info(const std::string &base_key, int uid,
                                UserInfo &userinfo) {
  std::string info_str;
  if (RedisMgr::getinstance()->get(base_key, info_str)) {
    Json::CharReaderBuilder reader_builder;
    std::unique_ptr<Json::CharReader> reader(reader_builder.newCharReader());
    Json::Value root;
    if (!reader->parse(info_str.c_str(), info_str.c_str() + info_str.size(),
                       &root, nullptr)) {
      std::cerr << "Failed to parse Redis JSON for key " << base_key << "\n";
      return false;
    }
    userinfo = json_to_userinfo(root);
  } else {
    auto find_res = MysqlMgr::getinstance()->get_user(uid);
    if (!find_res)
      return false;

    userinfo = std::move(find_res.value());
    RedisMgr::getinstance()->set(base_key,
                                 userinfo_to_json(userinfo).toStyledString());
  }
  std::cout << "user login uid: " << userinfo.uid << " name: " << userinfo.name
            << " email: " << userinfo.email << '\n';
  return true;
}

void LogicSystem::get_user_by_uid(const std::string &uid_str, int32_t uid,
                                  Json::Value &ret_value) {
  std::string base_key = std::string(REDIS_USER_BASE_INFO_PREFIX) + uid_str;

  std::string info_str;
  // 先去Redis查找用户信息
  if (RedisMgr::getinstance()->get(base_key, info_str)) {
    // Redis中有用户信息
    Json::CharReaderBuilder reader_builder;
    std::unique_ptr<Json::CharReader> reader(reader_builder.newCharReader());
    if (reader->parse(info_str.c_str(), info_str.c_str() + info_str.size(),
                      &ret_value, nullptr)) {
      ret_value["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR); // 成功
    } else {

      // 解析失败
      ret_value["error"] = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
    }
    return;
  }
  // Redis中没有，去MySQL查找
  auto find_res = MysqlMgr::getinstance()->get_user(uid);
  if (!find_res) {
    ret_value["error"] = static_cast<int32_t>(ErrorCodes::UID_INVALID);
    return;
  }

  auto &user_info = *find_res;
  Json::Value user_json = userinfo_to_json(user_info);

  // 热点数据写入 Redis
  Json::StreamWriterBuilder writer;
  writer["indentation"] = "";
  std::string json_str = Json::writeString(writer, user_json);
  RedisMgr::getinstance()->set(base_key, json_str);

  // 返回结果，同时带上 error=0
  ret_value = user_json;
  ret_value["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);
}

void LogicSystem::get_user_by_name(const std::string &name,
                                   Json::Value &rtvalue) {
  std::string base_key = std::string(REDIS_NAME_INFO_PREFIX) + name;

  std::string info_str;
  if (RedisMgr::getinstance()->get(base_key, info_str)) {
    // Redis中存在对应的用户信息
    Json::Reader reader;
    if (!reader.parse(info_str, rtvalue)) {
      rtvalue["error"] = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
    } else {

      rtvalue["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);
    }
    std::cout << "get user by name: " << rtvalue << '\n';
    return;
  }
  // Redis中没有，去MySQL查找
  auto find_res = MysqlMgr::getinstance()->get_user(name);
  if (find_res == std::nullopt) {
    rtvalue["error"] = static_cast<int32_t>(ErrorCodes::UID_INVALID);
    return;
  }

  auto &user_info = *find_res;
  Json::Value redis_root = userinfo_to_json(user_info);
  // 热点数据写入Redis
  Json::StreamWriterBuilder writer;
  writer["indentation"] = "";
  std::string json_str = Json::writeString(writer, redis_root);
  RedisMgr::getinstance()->set(base_key, json_str);

  rtvalue = redis_root;
  rtvalue["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);
}
