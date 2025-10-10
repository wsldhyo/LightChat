#include "chat_rpc_server.hpp"
#include "manager/mysql_manager.hpp"
#include "manager/redis_manager.hpp"
#include "session.hpp"
#include "user_manager.hpp"
#include "utility/constant.hpp"
#include "utility/defer.hpp"
#include "utility/userinfo.hpp"
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
ChatServiceImpl::ChatServiceImpl() {}

// 处理另一服务器转发的好友申请请求
Status ChatServiceImpl::NotifyAddFriend(ServerContext *context,
                                        const AddFriendReq *request,
                                        AddFriendRsp *response) {
  // 查找目标用户是否在当前服务器
  auto touid = request->touid();
  auto session = UserManager::getinstance()->get_session(touid);

  Defer defer([request, response]() {
    response->set_error(static_cast<int32_t>(ErrorCodes::NO_ERROR));
    response->set_applyuid(request->applyuid());
    response->set_touid(request->touid());
  });

  // 用户不在内存中，直接返回
  if (session == nullptr) {
    std::cout << "NotifyAddFriend: user not found: " << request->name() << '\n';
    return Status::OK;
  }

  // 构造好友申请通知 JSON
  Json::Value rtvalue;
  rtvalue["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);
  rtvalue["applyuid"] = request->applyuid();
  rtvalue["name"] = request->name();
  rtvalue["desc"] = request->desc();
  rtvalue["icon"] = request->icon();
  rtvalue["sex"] = request->sex();
  rtvalue["nick"] = request->nick();

  Json::StreamWriterBuilder writer;
  writer["indentation"] = ""; // 紧凑格式
  std::string return_str = Json::writeString(writer, rtvalue);

  std::cout << "NotifyAddFriend: data = " << return_str << '\n';
  session->send(return_str,
                static_cast<uint16_t>(ReqId::ID_NOTIFY_FRIEND_APPLY_REQ));

  return Status::OK;
}

// 处理另一服务器转发的好友认证结果
Status ChatServiceImpl::NotifyAuthFriend(ServerContext *context,
                                         const AuthFriendReq *request,
                                         AuthFriendRsp *response) {
  // 获取对端会话
  auto touid = request->touid();
  auto fromuid = request->fromuid();
  auto session = UserManager::getinstance()->get_session(touid);

  Defer defer([request, response]() {
    response->set_error(static_cast<int32_t>(ErrorCodes::NO_ERROR));
    response->set_fromuid(request->fromuid());
    response->set_touid(request->touid());
  });

  if (session == nullptr) {
    std::cout << "NotifyAuthFriend: user not found, touid=" << touid << '\n';
    return Status::OK;
  }

  // 构造好友认证结果通知 JSON
  Json::Value rtvalue;
  rtvalue["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);
  rtvalue["fromuid"] = fromuid;
  rtvalue["touid"] = touid;

  // 查询对方用户信息（fromuid， 被申请方）
  std::string base_key =
      std::string(REDIS_USER_BASE_INFO_PREFIX) + std::to_string(fromuid);
  auto user_info = std::make_shared<UserInfo>();
  bool b_info = GetBaseInfo(base_key, fromuid, user_info);
  if (b_info) {
    // 保存被申请方（认证方）的信息，准备转发给申请方
    rtvalue["name"] = user_info->name;
    rtvalue["nick"] = user_info->nick;
    rtvalue["icon"] = user_info->icon;
    rtvalue["sex"] = user_info->sex;
  } else {
    rtvalue["error"] = static_cast<int32_t>(ErrorCodes::UID_INVALID);
  }

  Json::StreamWriterBuilder writer;
  writer["indentation"] = ""; // 紧凑格式
  std::string return_str = Json::writeString(writer, rtvalue);

  std::cout << "NotifyAuthFriend: data = " << return_str << '\n';
  session->send(return_str,
                static_cast<uint16_t>(ReqId::ID_NOTIFY_AUTH_FRIEND_REQ));

  return Status::OK;
}

Status ChatServiceImpl::NotifyTextChatMsg(ServerContext *context,
                                          const TextChatMsgReq *request,
                                          TextChatMsgRsp *response) {
  auto touid = request->touid();
  auto session = UserManager::getinstance()->get_session(touid);

  Defer defer([response, request]() {
    response->set_error(static_cast<int32_t>(ErrorCodes::NO_ERROR));
  });

  if (session == nullptr) {
    std::cout << "NotifyTextChatMsg: user not found, touid=" << touid << '\n';
    return Status::OK;
  }

  // 构造文本消息通知 JSON
  Json::Value rtvalue;
  rtvalue["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);
  rtvalue["fromuid"] = request->fromuid();
  rtvalue["touid"] = request->touid();

  Json::Value text_array(Json::arrayValue);
  for (const auto &msg : request->textmsgs()) {
    Json::Value element;
    element["msgid"] = msg.msgid();
    element["content"] = msg.msgcontent();
    text_array.append(element);
  }
  rtvalue["text_array"] = text_array;

  Json::StreamWriterBuilder writer;
  writer["indentation"] = ""; // 紧凑格式
  std::string return_str = Json::writeString(writer, rtvalue);

  std::cout << "NotifyTextChatMsg: data = " << return_str << '\n';
  session->send(return_str,
                static_cast<uint16_t>(ReqId::ID_NOTIFY_TEXT_CHAT_MSG_REQ));

  return Status::OK;
}

bool ChatServiceImpl::GetBaseInfo(std::string base_key, int uid,
                                  std::shared_ptr<UserInfo> &userinfo) {
  // 尝试从 Redis 获取缓存的用户基础信息
  std::string info_str = "";
  bool b_base = RedisMgr::getinstance()->get(base_key, info_str);
  if (b_base) {
    // 从 JSON 中提取用户字段
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
    std::cout << "user base info from redis. user login uid is  "
              << userinfo->uid << " name  is " << userinfo->name << " pwd is "
              << userinfo->pwd << " email is " << userinfo->email << " icon is "
              << userinfo->icon << ".\n";
  } else {
    // Redis 缓存中没有，从 MySQL 查询用户信息
    auto user_info = MysqlMgr::getinstance()->get_user(uid);
    if (user_info == std::nullopt) {
      return false;
    }
    // 用查询结果构造 userinfo
    userinfo = std::make_shared<UserInfo>(user_info.value());
    Json::Value redis_root;
    redis_root["uid"] = uid;
    redis_root["pwd"] = userinfo->pwd;
    redis_root["name"] = userinfo->name;
    redis_root["email"] = userinfo->email;
    redis_root["nick"] = userinfo->nick;
    redis_root["desc"] = userinfo->desc;
    redis_root["sex"] = userinfo->sex;
    redis_root["icon"] = userinfo->icon;
    std::cout << "user base info from redis. user login uid is  "
              << userinfo->uid << " name  is " << userinfo->name << " pwd is "
              << userinfo->pwd << " email is " << userinfo->email << " icon is "
              << userinfo->icon << ".\n";
    // 存入Redis，以便下次直接通过Reids查找
    RedisMgr::getinstance()->set(base_key, redis_root.toStyledString());
  }

  return true;
}