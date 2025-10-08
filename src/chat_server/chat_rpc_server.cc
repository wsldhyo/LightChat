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
                                        AddFriendRsp *reply) {
  //查找用户是否在本服务器
  auto touid = request->touid();
  auto session = UserManager::getinstance()->get_session(touid);

  reply->set_error(static_cast<int32_t>(ErrorCodes::NO_ERROR));
  Defer defer([request, reply]() {
    reply->set_applyuid(request->applyuid());
    reply->set_touid(request->touid());
  });

  //用户不在内存中则直接返回
  if (session == nullptr) {
    std::cout << "rpc notify add friend.do not found the user:"
              << request->name() << '\n';
    return Status::OK;
  }

  //在内存中则直接转发申请信息给对方客户端
  // 构造Json
  Json::Value rtvalue;
  rtvalue["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);
  rtvalue["applyuid"] = request->applyuid();
  rtvalue["name"] = request->name();
  rtvalue["desc"] = request->desc();
  rtvalue["icon"] = request->icon();
  rtvalue["sex"] = request->sex();
  rtvalue["nick"] = request->nick();

  // 序列化为字符串
  Json::StreamWriterBuilder writer;
  writer["indentation"] = ""; // 紧凑格式
  std::string return_str = Json::writeString(writer, rtvalue);
  std::cout << "rpc notify add friend apply.data is:\n" << return_str << '\n';
  // 发送
  session->send(return_str,
                static_cast<uint16_t>(ReqId::ID_NOTIFY_FRIEND_APPLY_REQ));

  return Status::OK;
}

// 处理另一服务器转发的好友认证结果
Status ChatServiceImpl::NotifyAuthFriend(ServerContext *context,
                                         const AuthFriendReq *request,
                                         AuthFriendRsp *response) {
  auto touid = request->touid();
  auto fromuid = request->fromuid();
  auto session = UserManager::getinstance()->get_session(touid);

  Defer defer([request, response]() {
    response->set_error(static_cast<int32_t>(ErrorCodes::NO_ERROR));
    response->set_fromuid(request->fromuid());
    response->set_touid(request->touid());
  });

  if (session == nullptr) {
    return Status::OK;
  }

  Json::Value rtvalue;
  rtvalue["error"] = static_cast<int32_t>(ErrorCodes::NO_ERROR);
  rtvalue["fromuid"] = request->fromuid();
  rtvalue["touid"] = request->touid();

  std::string base_key =
      std::string(REDIS_USER_BASE_INFO_PREFIX) + std::to_string(fromuid);
  auto user_info = std::make_shared<UserInfo>();
  bool b_info = GetBaseInfo(base_key, fromuid, user_info);
  if (b_info) {
    rtvalue["name"] = user_info->name;
    rtvalue["nick"] = user_info->nick;
    rtvalue["icon"] = user_info->icon;
    rtvalue["sex"] = user_info->sex;
  } else {
    rtvalue["error"] = static_cast<int32_t>(ErrorCodes::UID_INVALID);
  }

  std::string return_str = rtvalue.toStyledString();

  session->send(return_str,
                static_cast<int32_t>(ReqId::ID_NOTIFY_AUTH_FRIEND_REQ));
  return Status::OK;
}

Status ChatServiceImpl::NotifyTextChatMsg(::grpc::ServerContext *context,
                                          const TextChatMsgReq *request,
                                          TextChatMsgRsp *response) {
  return Status::OK;
}

bool ChatServiceImpl::GetBaseInfo(std::string base_key, int uid,
                                  std::shared_ptr<UserInfo> &userinfo) {
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
    std::cout << "user base info from redis. user login uid is  " << userinfo->uid << " name  is "
              << userinfo->name << " pwd is " << userinfo->pwd << " email is "
              << userinfo->email << " icon is " << userinfo->icon << ".\n";
  } else {
    auto user_info = MysqlMgr::getinstance()->get_user(uid);
    if (user_info == std::nullopt) {
      return false;
    }

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
    std::cout << "user base info from redis. user login uid is  " << userinfo->uid << " name  is "
              << userinfo->name << " pwd is " << userinfo->pwd << " email is "
              << userinfo->email << " icon is " << userinfo->icon << ".\n";
    RedisMgr::getinstance()->set(base_key, redis_root.toStyledString());
  }

  return true;
}