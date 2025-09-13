#ifndef CHAT_RPC_SERVER_HPP
#define CHAT_RPC_SERVER_HPP
#include "message.grpc.pb.h"
#include "message.pb.h"
using grpc::ServerContext;
using grpc::Status;
using message::AddFriendReq;
using message::AddFriendRsp;
using message::AuthFriendReq;
using message::AuthFriendRsp;
using message::ChatService;
using message::TextChatMsgReq;
using message::TextChatMsgRsp;
class UserInfo;

class ChatServiceImpl final : public ChatService::Service {
public:
  ChatServiceImpl();
  /**
   * @brief 处理好友申请的通知请求。
   *
   * gRPC 服务端接口：接收 AddFriendReq 请求并填充 AddFriendRsp 响应。
   * - 根据 to_uid 查找用户是否在当前服务器的内存会话中：
   *   - 若未找到，则直接返回（表示对方未登录本服务器）；
   *   - 若找到，则构造好友申请的 JSON 数据并通过用户会话转发给目标客户端。
   *
   * @param context  gRPC 服务端上下文。
   * @param request  好友申请请求，包含申请人信息与目标用户 ID。
   * @param reply    响应对象，函数内填充错误码及必要字段。
   *
   * @return grpc::Status 固定返回 Status::OK（逻辑错误通过 reply.error 表示）。
   *
   * @note
   * - 当用户不在线时不会发送消息，仅返回成功状态。
   * - 使用 RAII（Defer）确保响应中始终包含必要的标识字段。
   */

  Status NotifyAddFriend(ServerContext *context, const AddFriendReq *request,
                         AddFriendRsp *reply) override;

  Status NotifyAuthFriend(ServerContext *context, const AuthFriendReq *request,
                          AuthFriendRsp *response) override;

  Status NotifyTextChatMsg(::grpc::ServerContext *context,
                           const TextChatMsgReq *request,
                           TextChatMsgRsp *response) override;

  bool GetBaseInfo(std::string base_key, int uid,
                   std::shared_ptr<UserInfo> &userinfo);

private:
};

#endif // CHAT_RPC_SERVER_HPP