#include "logic_system.hpp"
#include "manager/config_manager.hpp"
#include "manager/mysql_manager.hpp"
#include "manager/redis_manager.hpp"
#include "rpc_client/chat_rpc_client.hpp"
#include "rpc_client/status_rpc_client.hpp"
#include "server.hpp"
#include "session.hpp"
#include "user_manager.hpp"
#include "utility/constant.hpp"
#include "utility/defer.hpp"
#include "utility/toolfunc.hpp"
#include "utility/userinfo.hpp"
#include <iostream>
#include <json/reader.h>
#include <json/writer.h>

LogicSystem::LogicSystem() : b_stop_(false), server_(nullptr) {
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

void LogicSystem::set_server(std::shared_ptr<Server> server) {
  server_ = server;
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
        std::cout << "recv msg:\n" << logic_node.msg_->data_ << '\n';
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
  // =======================================
  // 注册登录聊天服务器请求
  // =======================================
  handlers_.emplace(
      ReqId::ID_CHAT_LOGIN_REQ,
      [this](std::shared_ptr<Session> session, ReqId, std::string_view msg) {
        Json::Value root;
        Json::Value rtvalue;

        // 使用 Defer 确保返回响应
        Defer defer([session, &rtvalue]() {
          session->send(json_compact(rtvalue),
                        static_cast<uint16_t>(ReqId::ID_CHAT_LOGIN_RSP));
        });
        // 解析客户端发送的 JSON 消息
        if (!parse_json(msg, root)) {
          // JSON 解析失败，返回解析错误
          rtvalue["error"] =
              static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
          return;
        }

        // 从 JSON 中获取用户 ID 和 token
        int uid = root["uid"].asInt();
        auto uid_str = root["uid"].asString();
        std::string token = root["token"].asString();
        std::cout << "user login uid: " << uid << ", token: " << token
                  << std::endl;

        // TODO，直接在这里验证Token，不必走状态服务器
        // 调用 gRPC
        // 接口验证用户身份(验证用户传递的Token和Redis中存储的是否一致)
        auto rsp = StatusGrpcClient::get_instance()->login(uid, token);
        if (rsp.error() != static_cast<int>(ErrorCodes::NO_ERROR)) {
          rtvalue["error"] = rsp.error();
          return;
        }

        // 获取用户基础信息
        UserInfo user_info;
        std::string base_key =
            std::string(REDIS_USER_BASE_INFO_PREFIX) + uid_str;
        if (!get_base_info(base_key, uid, user_info)) {
          rtvalue["error"] = static_cast<int>(ErrorCodes::UID_INVALID);
          return;
        }

        //此处添加分布式锁，让该线程独占登录
        // ***
        // 先加分布式锁，保证对Redis的操作不受污染。TODO，锁粒度较大，可对一些允许接受旧值数据访问移除到锁保护范围外
        //拼接用户ip对应的key
        auto lock_key = std::string(REDIS_LOCK_PREFIX) + uid_str;
        auto identifier = RedisMgr::get_instance()->acquire_lock(
            lock_key, REDIS_LOCK_TIMEOUT, REDIS_ACQUIRE_TIMEOUT);
        //利用defer解锁
        Defer defer2([this, identifier, lock_key]() {
          RedisMgr::get_instance()->release_lock(lock_key, identifier);
        });
        // 先查询用户是否登录（Redis中记录了uid的登录ip），如果已经登录，则需要进行踢人处理
        std::string uid_ip_value = "";
        auto uid_ip_key = std::string(REDIS_USER_IP_PREFIX) + uid_str;
        bool b_ip = RedisMgr::get_instance()->get(uid_ip_key, uid_ip_value);
        if (b_ip) {
          // 用户已经登录
          //获取当前服务器ip信息
          auto cfg = ConfigManager::get_instance();
          auto self_name = (*cfg)["SelfServer"]["name"];
          //如果之前登录的服务器和当前相同，则直接在本服务器踢掉
          if (uid_ip_value == self_name) {
            // 在同一服务器登录，直接清除旧的会话

            //查找旧有的连接
            auto old_session = UserManager::get_instance()->get_session(uid);

            //此处应该发送踢人消息
            if (old_session) {
              old_session->notify_offline(uid);
              //清除旧的连接
              server_->remove_session(old_session->get_session_id());
            }

          } else {
            //跨服踢人，如果不是本服务器，则通知grpc通知其他服务器踢掉
            //如果不是本服务器，则通知grpc通知其他服务器踢掉
            //发送通知
            message::KickUserReq kick_req;
            kick_req.set_uid(uid);
            ChatGrpcClient::get_instance()->NotifyKickUser(uid_ip_value,
                                                           kick_req);
          }
        }

        // 构造返回 JSON 包含用户信息和好友信息
        rtvalue = userinfo_to_json(user_info);
        get_friend_apply_info(uid, rtvalue);
        get_friend_list(uid, rtvalue);

        // 更新当前服务器登录计数
        auto cfg = ConfigManager::get_instance();
        auto server_name = cfg->get_value("SelfServer", "name");
        int count = 0;
        auto login_count = RedisMgr::get_instance()->h_get(
            REDIS_LOGIN_COUNT_PREFIX, server_name);
        RedisMgr::get_instance()->increase_count(server_name);
        // 缓存用户 session 和所在服务器信息
        session->set_user_id(uid);
        RedisMgr::get_instance()->set(std::string(REDIS_USER_IP_PREFIX) +
                                          std::to_string(uid),
                                      server_name);
        UserManager::get_instance()->set_user_session(uid, session);

        // 登录成功返回 NO_ERROR
        rtvalue["error"] = static_cast<int>(ErrorCodes::NO_ERROR);
      });
  // =======================================
  // 搜索用户请求
  // =======================================
  handlers_.emplace(
      ReqId::ID_SEARCH_USER_REQ,
      [this](std::shared_ptr<Session> session, ReqId, std::string_view msg) {
        Json::Value root, rtvalue;

        // 使用 Defer 确保返回响应
        Defer defer([session, &rtvalue]() {
          session->send(json_compact(rtvalue),
                        static_cast<uint16_t>(ReqId::ID_SEARCH_USER_RSP));
        });
        // 解析客户端 JSON 请求
        if (!parse_json(msg, root)) {
          rtvalue["error"] =
              static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
          return;
        }

        std::string uid_str = root["uid"].asString();
        std::cout << "User Searchinfo uid: " << uid_str << std::endl;

        // 判断输入是数字 uid 还是用户名
        int32_t uid = 0;
        auto ec = string_to_int(uid_str, uid);
        if (ec == ErrorCodes::NO_ERROR)
          get_user_by_uid(uid_str, uid, rtvalue);
        else
          get_user_by_name(uid_str, rtvalue);
      });
  // =======================================
  // 好友申请请求
  // =======================================
  handlers_.emplace(
      ReqId::ID_APPLY_FRIEND_REQ,
      [this](std::shared_ptr<Session> session, ReqId, std::string_view msg) {
        Json::Value root, rtvalue;

        // 使用 Defer 确保返回响应
        Defer defer([session, &rtvalue]() {
          session->send(json_compact(rtvalue),
                        static_cast<uint16_t>(ReqId::ID_APPLY_FRIEND_RSP));
        });
        // 解析 JSON 请求
        if (!parse_json(msg, root)) {
          rtvalue["error"] =
              static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
          return;
        }

        rtvalue["error"] = static_cast<int>(ErrorCodes::NO_ERROR);
        // 获取请求中的申请信息
        int uid = root["uid"].asInt();
        std::string applyname = root["applyname"].asString();
        std::string bakname = root["bakname"].asString();
        int touid = root["touid"].asInt();
        std::cout << "user apply uid=" << uid << " to=" << touid << std::endl;

        // 将好友申请写入数据库
        MysqlMgr::get_instance()->add_friend_apply(uid, touid);

        // 查询目标用户是否在线
        std::string to_ip_key =
            std::string(REDIS_USER_IP_PREFIX) + std::to_string(touid);
        std::string to_ip_value;
        if (!RedisMgr::get_instance()->get(to_ip_key, to_ip_value)) {
          std::cout << "The other user has not logged in." << std::endl;
          return;
        }

        // 获取申请用户信息
        auto cfg = ConfigManager::get_instance();
        auto self_name = cfg->get_value("SelfServer", "name");
        std::string base_key =
            std::string(REDIS_USER_BASE_INFO_PREFIX) + std::to_string(uid);
        UserInfo apply_info;
        bool b_info = get_base_info(base_key, uid, apply_info);

        // 如果目标用户在同一服务器，直接发送通知
        if (to_ip_value == self_name) {
          if (auto peer_session =
                  UserManager::get_instance()->get_session(touid)) {
            Json::Value notify;
            notify["applyuid"] = uid;
            notify["name"] = applyname;
            if (b_info) {
              notify["icon"] = apply_info.icon;
              notify["sex"] = apply_info.sex;
              notify["nick"] = apply_info.nick;
            }
            peer_session->send(
                json_compact(notify),
                static_cast<uint16_t>(ReqId::ID_NOTIFY_FRIEND_APPLY_REQ));
          }
          return;
        }

        // 否则通过 gRPC 通知目标用户所在服务器
        AddFriendReq add_req;
        add_req.set_applyuid(uid);
        add_req.set_touid(touid);
        add_req.set_name(applyname);
        if (b_info) {
          add_req.set_icon(apply_info.icon);
          add_req.set_sex(apply_info.sex);
          add_req.set_nick(apply_info.nick);
        }
        ChatGrpcClient::get_instance()->notify_add_friend(to_ip_value, add_req);
      });
  // =======================================
  // 好友认证请求
  // =======================================
  handlers_.emplace(
      ReqId::ID_AUTH_FRIEND_REQ,
      [this](std::shared_ptr<Session> session, ReqId, std::string_view msg) {
        Json::Value root, rtvalue;
        // 使用 Defer 确保返回响应
        Defer defer([session, &rtvalue]() {
          session->send(json_compact(rtvalue),
                        static_cast<uint16_t>(ReqId::ID_AUTH_FRIEND_RSP));
        });
        // 解析 JSON 请求
        if (!parse_json(msg, root)) {
          rtvalue["error"] =
              static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
          ;
          return;
        }
        // 获取请求中的好友认证信息
        int uid = root["fromuid"].asInt();
        int touid = root["touid"].asInt();
        std::string back_name = root["back"].asString();

        std::cout << "from " << uid << " auth friend to " << touid << std::endl;

        rtvalue["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);
        UserInfo user_info{};
        std::string base_key =
            std::string(REDIS_USER_BASE_INFO_PREFIX) + std::to_string(touid);
        bool b_info = get_base_info(base_key, touid, user_info);
        if (b_info) {
          rtvalue = userinfo_to_json(user_info);
          rtvalue["uid"] = touid;
        } else {
          rtvalue["error"] = static_cast<int32_t>(ErrorCodes::UID_INVALID);
        }
        // 更新数据库认证申请和好友关系
        MysqlMgr::get_instance()->auth_friend_apply(uid, touid);
        MysqlMgr::get_instance()->add_friend(uid, touid, back_name);

        // 查询目标用户是否在线
        std::string to_ip_key =
            std::string(REDIS_USER_IP_PREFIX) + std::to_string(touid);
        std::string to_ip_value;
        if (!RedisMgr::get_instance()->get(to_ip_key, to_ip_value))
          return;

        auto cfg = ConfigManager::get_instance();
        auto self_name = cfg->get_value("SelfServer", "name");

        // 如果目标用户在同一服务器，直接发送通知
        if (to_ip_value == self_name) {
          if (auto peer = UserManager::get_instance()->get_session(touid)) {
            Json::Value notify;
            notify["error"] = static_cast<int>(ErrorCodes::NO_ERROR);
            notify["fromuid"] = uid;
            notify["touid"] = touid;

            UserInfo info;
            if (get_base_info(std::string(REDIS_USER_BASE_INFO_PREFIX) +
                                  std::to_string(uid),
                              uid, info)) {
              notify["name"] = info.name;
              notify["nick"] = info.nick;
              notify["icon"] = info.icon;
              notify["sex"] = info.sex;
            } else {
              notify["error"] = static_cast<int>(ErrorCodes::UID_INVALID);
            }
            peer->send(json_compact(notify),
                       static_cast<uint16_t>(ReqId::ID_NOTIFY_AUTH_FRIEND_REQ));
          }
          return;
        }

        // 否则通过 gRPC 通知目标用户所在服务器
        AuthFriendReq auth_req;
        auth_req.set_fromuid(uid);
        auth_req.set_touid(touid);
        ChatGrpcClient::get_instance()->notify_auth_friend(to_ip_value,
                                                           auth_req);
      });
  // =======================================
  // 聊天消息请求
  // =======================================
  handlers_.emplace(
      ReqId::ID_TEXT_CHAT_MSG_REQ,
      [this](std::shared_ptr<Session> session, ReqId, std::string_view msg) {
        Json::Value root, rtvalue;

        // 使用 Defer 确保返回响应
        Defer defer([session, &rtvalue]() {
          session->send(json_compact(rtvalue),
                        static_cast<uint16_t>(ReqId::ID_TEXT_CHAT_MSG_RSP));
        });
        // 解析客户端 JSON 消息
        if (!parse_json(msg, root)) {
          rtvalue["error"] = static_cast<int>(ErrorCodes::PARSE_JSON_FAILED);
          return;
        }

        // 获取发送者、接收者和消息数组
        int uid = root["fromuid"].asInt();
        int touid = root["touid"].asInt();
        const Json::Value &arrays = root["text_array"];

        // 构造返回 JSON
        rtvalue["error"] = static_cast<int>(ErrorCodes::NO_ERROR);
        rtvalue["text_array"] = arrays;
        rtvalue["fromuid"] = uid;
        rtvalue["touid"] = touid;

        // 查询目标用户所在服务器
        std::string to_ip_value;
        if (!RedisMgr::get_instance()->get(std::string(REDIS_USER_IP_PREFIX) +
                                               std::to_string(touid),
                                           to_ip_value))
          return;

        auto cfg = ConfigManager::get_instance();
        auto self_name = cfg->get_value("SelfServer", "name");

        // 如果目标用户在同一服务器，直接发送消息
        if (to_ip_value == self_name) {
          if (auto peer = UserManager::get_instance()->get_session(touid)) {
            peer->send(
                json_compact(rtvalue),
                static_cast<uint16_t>(ReqId::ID_NOTIFY_TEXT_CHAT_MSG_REQ));
          }
          return;
        }

        // 否则通过 gRPC 转发消息到目标用户所在服务器
        TextChatMsgReq text_req;
        text_req.set_fromuid(uid);
        text_req.set_touid(touid);
        for (const auto &txt_obj : arrays) {
          auto *msg_ptr = text_req.add_textmsgs();
          msg_ptr->set_msgid(txt_obj["msgid"].asString());
          msg_ptr->set_msgcontent(txt_obj["content"].asString());
        }
        ChatGrpcClient::get_instance()->notify_text_chat_msg(to_ip_value,
                                                             text_req, rtvalue);
      });
}

bool LogicSystem::get_base_info(const std::string &base_key, int uid,
                                UserInfo &userinfo) {
  std::string info_str;
  //  Redis中的可能是旧的，先不从Redis中取数据
  if (RedisMgr::get_instance()->get(base_key, info_str)) {
    Json::CharReaderBuilder reader_builder;
    std::unique_ptr<Json::CharReader> reader(reader_builder.newCharReader());
    Json::Value root;
    std::cout << "Query user base info from redis:" << info_str << '\n';
    if (!reader->parse(info_str.c_str(), info_str.c_str() + info_str.size(),
                       &root, nullptr)) {
      std::cerr << "Failed to parse Redis JSON for key " << base_key << "\n";
      return false;
    }
    userinfo = json_to_userinfo(root);

    std::cout << "user info from redis. user login uid is  " << userinfo.uid
              << " name  is " << userinfo.name << " pwd is " << userinfo.pwd
              << " email is " << userinfo.email << " icon is " << userinfo.icon
              << ".\n";
  } else {
    auto find_res = MysqlMgr::get_instance()->get_user(uid);
    if (!find_res)
      return false;

    userinfo = std::move(find_res.value());
    // 将用户的Base Info 写入Redis
    RedisMgr::get_instance()->set(base_key,
                                  userinfo_to_json(userinfo).toStyledString());

    std::cout << "set user info to redis. user login uid is  " << userinfo.uid
              << " name  is " << userinfo.name << " pwd is " << userinfo.pwd
              << " email is " << userinfo.email << " icon is " << userinfo.icon
              << ".\n";
  }
  return true;
}

void LogicSystem::get_user_by_uid(const std::string &uid_str, int32_t uid,
                                  Json::Value &rtvalue) {
  std::string base_key = std::string(REDIS_USER_BASE_INFO_PREFIX) + uid_str;

  std::string info_str;
  // 先去Redis查找用户信息
  if (RedisMgr::get_instance()->get(base_key, info_str)) {
    // Redis中有用户信息
    Json::CharReaderBuilder reader_builder;
    std::unique_ptr<Json::CharReader> reader(reader_builder.newCharReader());
    if (reader->parse(info_str.c_str(), info_str.c_str() + info_str.size(),
                      &rtvalue, nullptr)) {
      rtvalue["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR); // 成功
    } else {

      // 解析失败
      rtvalue["error"] = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
    }
    return;
  }
  // Redis中没有，去MySQL查找
  auto find_res = MysqlMgr::get_instance()->get_user(uid);
  if (!find_res) {
    rtvalue["error"] = static_cast<int32_t>(ErrorCodes::UID_INVALID);
    return;
  }

  auto &user_info = *find_res;
  Json::Value user_json = userinfo_to_json(user_info);

  // 热点数据写入 Redis
  Json::StreamWriterBuilder writer;
  writer["indentation"] = ""; // 紧凑格式
  std::string json_str = Json::writeString(writer, user_json);
  RedisMgr::get_instance()->set(base_key, json_str);
  std::cout << "set user base info to redis@" << json_str << '\n';
  // 返回结果，同时带上 error=0
  rtvalue = user_json;
  rtvalue["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);
}

void LogicSystem::get_user_by_name(const std::string &name,
                                   Json::Value &rtvalue) const {
  std::string base_key = std::string(REDIS_NAME_INFO_PREFIX) + name;

  std::string info_str;
  if (RedisMgr::get_instance()->get(base_key, info_str)) {
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
  auto find_res = MysqlMgr::get_instance()->get_user(name);
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
  RedisMgr::get_instance()->set(base_key, json_str);

  rtvalue = redis_root;
  rtvalue["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);
}

void LogicSystem::get_friend_apply_info(int const to_uid,
                                        Json::Value &rtvalue) const {

  std::vector<std::shared_ptr<ApplyInfo>> apply_list;
  if (MysqlMgr::get_instance()->get_apply_list(to_uid, apply_list, 0, 10)) {
    std::cout << "add apply freind list.\n";
    // 将好友申请列表中的信息序列化为Json
    Json::Value arr(Json::arrayValue);
    arr.resize(apply_list.size()); // 直接预分配空间
    // 序列为Json
    for (size_t i = 0; i < apply_list.size(); ++i) {
      arr[(int)i] =
          applyinfo_to_json(*apply_list[i]); // 将单个好友申请的Json添加到arr中
    }
    rtvalue["apply_list"] = std::move(arr); // 将所有好友好友添加到返回Json中
  }
}

bool LogicSystem::get_friend_list(int self_id, Json::Value &rtvalue) {
  // 从数据库查询好友列表

  std::vector<std::shared_ptr<UserInfo>> friend_list;
  if (MysqlMgr::get_instance()->get_friend_list(self_id, friend_list)) {
    // 将好友列表对象序列化为Json
    for (auto &friend_ele : friend_list) {
      Json::Value obj;
      obj["name"] = friend_ele->name;
      obj["uid"] = friend_ele->uid;
      obj["icon"] = friend_ele->icon;
      obj["nick"] = friend_ele->nick;
      obj["sex"] = friend_ele->sex;
      obj["desc"] = friend_ele->desc;
      obj["back"] = friend_ele->back;
      rtvalue["friend_list"].append(obj);
    }
    return true;
  }
  return false;
}