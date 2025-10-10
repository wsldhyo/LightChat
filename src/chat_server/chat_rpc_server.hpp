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
  /**
   * @brief 通知目标用户有新的好友申请。
   *
   * 该函数由 RPC 框架调用，用于当某个用户向另一用户发送好友申请时，
   * 将好友申请消息转发给目标用户所在服务器的 TCP 会话。如果目标用户
   * 当前未在本服务器内存中（即未在线或未连接），则直接返回成功状态。
   *
   * @param context  gRPC 服务上下文（未使用）
   * @param request  包含好友申请方信息（applyuid、name、desc、icon、sex、nick
   * 等）
   * @param reply    RPC 回复对象，函数结束时通过 Defer 自动填充基本返回字段
   *
   * @return Status 始终返回 Status::OK 表示 RPC 调用成功（逻辑错误通过 JSON
   * 通知体现）
   */
  Status NotifyAuthFriend(ServerContext *context, const AuthFriendReq *request,
                          AuthFriendRsp *response) override;
  /**
   * @brief 向目标用户通知一条文本聊天消息。
   *
   * 该函数由 gRPC 服务调用，用于将一方用户发送的文本消息转发给接收方用户。
   * 如果接收方当前在线（存在 session），则会直接通过会话通道推送消息；
   * 否则仅返回 OK 状态，不执行推送。
   *
   * @param context  gRPC 上下文对象。
   * @param request  请求对象，包含发送者 UID、接收者 UID、文本消息数组等信息。
   * @param response 响应对象，用于返回错误码或其他信息给调用方。
   *
   * @return grpc::Status 永远返回
   * Status::OK，因为消息通知不依赖于对方在线状态。
   */
  Status NotifyTextChatMsg(::grpc::ServerContext *context,
                           const TextChatMsgReq *request,
                           TextChatMsgRsp *response) override;

  /**
   * @brief 获取指定用户的基础信息。
   *
   * 优先从 Redis 中获取用户信息，若缓存中不存在，则从 MySQL 查询并回填到
   * Redis。 成功后通过 `userinfo` 参数返回完整的用户数据结构。
   *
   * @param base_key  Redis 缓存键。
   * @param uid       用户ID。
   * @param userinfo  输出参数，返回用户的基础信息结构。
   *
   * @return true  成功获取到用户信息（来自 Redis 或 MySQL）。
   * @return false 查询失败（MySQL 中无此用户）。
   */
  bool GetBaseInfo(std::string base_key, int uid,
                   std::shared_ptr<UserInfo> &userinfo);

private:
};

#endif // CHAT_RPC_SERVER_HPP