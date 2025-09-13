#include "chat_rpc_server.hpp"
#include "session.hpp"
#include "user_manager.hpp"
#include "utility/constant.hpp"
#include "utility/defer.hpp"
#include "utility/userinfo.hpp"
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

Status ChatServiceImpl::NotifyAuthFriend(ServerContext *context,
                                         const AuthFriendReq *request,
                                         AuthFriendRsp *response) {
  return Status::OK;
}

Status ChatServiceImpl::NotifyTextChatMsg(::grpc::ServerContext *context,
                                          const TextChatMsgReq *request,
                                          TextChatMsgRsp *response) {
  return Status::OK;
}

bool ChatServiceImpl::GetBaseInfo(std::string base_key, int uid,
                                  std::shared_ptr<UserInfo> &userinfo) {
  return true;
}