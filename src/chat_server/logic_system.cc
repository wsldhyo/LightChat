#include "logic_system.hpp"
#include "manager/config_manager.hpp"
#include "manager/mysql_manager.hpp"
#include "manager/redis_manager.hpp"
#include "rpc_client/chat_rpc_client.hpp"
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
  handlers_.emplace(
      ReqId::ID_CHAT_LOGIN_REQ,
      [this](std::shared_ptr<Session> session, ReqId id, std::string_view msg) {
        Json::CharReaderBuilder reader_builder;
        std::unique_ptr<Json::CharReader> reader(
            reader_builder.newCharReader());
        Json::Value root;
        if (!reader->parse(msg.data(), msg.data() + msg.size(), &root,
                           nullptr)) {
          Json::Value rsp_json;
          rsp_json["error"] =
              static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
          session->send(rsp_json.toStyledString(),
                        static_cast<uint16_t>(ReqId::ID_CHAT_LOGIN_RSP));
          return;
        }

        int uid = root["uid"].asInt();
        std::string token = root["token"].asString();
        std::cout << "user login uid: " << uid << ", token: " << token
                  << std::endl;

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
        auto login_count = RedisMgr::getinstance()->h_get(
            REDIS_LOGIN_COUNT_PREFIX, server_name);
        int count = 0;
        if (!login_count.empty()) {
          string_to_int(login_count, count);
        }
        ++count;
        RedisMgr::getinstance()->h_set(REDIS_LOGIN_COUNT_PREFIX, server_name,
                                       std::to_string(count));

        // session绑定uid，缓存登录IP和session
        session->set_user_id(uid);
        //缓存登录的ip和所在服务器，方便后续两聊天服务器查找对端聊天服务器
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
  // 处理好友申请请求
  handlers_.emplace(ReqId::ID_APPLY_FRIEND_REQ, [this](std::shared_ptr<Session>
                                                           session,
                                                       ReqId id,
                                                       std::string_view msg) {
    // 解析请求数据
    Json::CharReaderBuilder reader_builder;
    std::unique_ptr<Json::CharReader> reader(reader_builder.newCharReader());
    Json::Value root;
    Json::Value rtvalue;

    rtvalue["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);

    Defer defer([this, &rtvalue, session]() {
      // 通知申请方客户端，
      std::string return_str = rtvalue.toStyledString();
      session->send(return_str,
                    static_cast<int32_t>(ReqId::ID_APPLY_FRIEND_RSP)); // 通知申请方的客户端，服务器处理申请结果
    });
    if (!reader->parse(msg.data(), msg.data() + msg.size(), &root, nullptr)) {
      std::cout << "apply friend req, json parse error\n";
      rtvalue["error"] = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
      // 请求数据错误，直接给申请方答复，不需要往下通知对端
      return;
    }
    auto uid = root["uid"].asInt();
    auto applyname = root["applyname"].asString();
    auto bakname = root["bakname"].asString();
    auto touid = root["touid"].asInt();
    std::cout << "user login uid is  " << uid << " applyname  is " << applyname
              << " bakname is " << bakname << " touid is " << touid << '\n';

    //先更新数据库
    MysqlMgr::getinstance()->add_friend_apply(uid, touid);

    // 查找登录的服务器， 该信息在用户登录聊天服务器时会缓存到Redis
    auto to_str = std::to_string(touid);
    auto to_ip_key = std::string(REDIS_USER_IP_PREFIX) + to_str;
    std::string to_ip_value = "";
    bool b_ip = RedisMgr::getinstance()->get(to_ip_key, to_ip_value);
    if (!b_ip) {
      // 对方没有登录聊天服务器，则直接返回。 TODO 等对方登录后转发
      std::cout << "The other user has not logged into the server.\n";
      return;
    }
    // 对方已登录
    auto cfg = ConfigManager::getinstance();
    auto self_name = (*cfg)["SelfServer"]["name"];

    //如果双方在同一个聊天服务器登录，直接通知对方有申请消息
    if (to_ip_value == self_name) {
      // 取出对方uid的会话，并通过该会话将消息转发给对方客户端
      auto session = UserManager::getinstance()->get_session(touid);
      if (session) {
        //在内存中则直接发送通知对方
        Json::Value notify;
        notify["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);
        notify["applyuid"] = uid;
        notify["name"] = applyname;
        notify["desc"] = "";
        std::string return_str = notify.toStyledString();
        std::cout << "Both users are logged into the same server\n";
        session->send(
            return_str,
            static_cast<int32_t>(
                ReqId::ID_NOTIFY_FRIEND_APPLY_REQ)); // 通知被申请方有好友申请到达
      }

      return;
    }
    // 对方登录的聊天服务器不在本服务器，转发好友申请到对方服务器
    std::cout << "The other user has logined into another server.\n";
    std::string base_key =
        std::string(REDIS_USER_BASE_INFO_PREFIX) + std::to_string(uid);
    UserInfo apply_info;
    bool b_info = get_base_info(base_key, uid, apply_info);
    // 通过GRPC调用，通知对方服务器
    AddFriendReq add_req; // 需传递的参数
    add_req.set_applyuid(uid);
    add_req.set_touid(touid);
    add_req.set_name(applyname);
    add_req.set_desc("");
    if (b_info) {
      add_req.set_icon(apply_info.icon);
      add_req.set_sex(apply_info.sex);
      add_req.set_nick(apply_info.nick);
    }

    //发送通知
    ChatGrpcClient::getinstance()->notify_add_friend(to_ip_value, add_req);
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
    // 将用户的Base Info 写入Redis
    RedisMgr::getinstance()->set(base_key,
                                 userinfo_to_json(userinfo).toStyledString());
  }
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
  writer["indentation"] = ""; // 紧凑格式
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
