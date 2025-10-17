#ifndef CHAT_RPC_CLIENT_HPP
#define CHAT_RPC_CLIENT_HPP
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "utility/singleton.hpp"
#include <json/value.h>
#include <unordered_map>
class ChatServerConnPool;
using message::AddFriendReq;
using message::AddFriendRsp;
using message::AuthFriendReq;
using message::AuthFriendRsp;
using message::KickUserReq;
using message::KickUserRsp;
using message::TextChatMsgReq;
using message::TextChatMsgRsp;
using server_name_t = std::string;
class UserInfo;
class ChatGrpcClient : public Singleton<ChatGrpcClient> {
  friend class Singleton<ChatGrpcClient>;

public:
  ~ChatGrpcClient();
  /**
   * @brief  向指定服务器发送“添加好友”通知请求。
   *
   * 该函数通过 gRPC 调用远端服务器的 NotifyAddFriend 接口，
   * 并将请求结果封装在 AddFriendRsp 中返回。
   * 在函数作用域结束时，会自动填充响应中的 applyuid 和 touid 字段，
   * 并归还连接池中的连接。
   *
   * @param server_ip  目标服务器的 IP 地址，用于查找对应的连接池。
   * @param req        添加好友的请求参数，包含申请方和接收方信息。
   *
   * @return AddFriendRsp
   *         - error 字段为 ErrorCodes::NO_ERROR 表示调用成功；
   *         - error 字段为 ErrorCodes::RPC_CALL_FAILED 表示 RPC 调用失败；
   *         - 其他错误码根据业务逻辑扩展。
   *
   * @note
   * -
   * 如果未找到对应的服务器连接池，会直接返回默认的响应对象（error=NO_ERROR，但未实际调用
   * RPC）。
   * - 使用
   * RAII（Defer）确保在函数退出时自动设置必要的响应字段，并归还连接池连接。
   */

  AddFriendRsp notify_add_friend(std::string server_ip,
                                 const AddFriendReq &req);
  /**
   * @brief 通知指定服务器执行好友认证操作
   *
   * 通过 gRPC 调用目标服务器的 NotifyAuthFriend 接口，
   * 用于处理好友请求的授权或确认逻辑。
   * 若目标服务器连接不可用或 RPC 调用失败，则返回错误码。
   *
   * @param server_ip  目标服务器 IP 地址
   * @param req        好友认证请求对象
   * @return AuthFriendRsp  响应结果，包含错误码及双方 UID
   */
  AuthFriendRsp notify_auth_friend(std::string server_ip,
                                   const AuthFriendReq &req);
  /**
   * @brief 获取用户基础信息（优先从 Redis 缓存获取）
   *
   * 该函数先尝试从 Redis 获取用户信息；
   * 若缓存不存在，则从 MySQL 读取用户信息，
   * 并将结果回写到 Redis 缓存中。
   *
   * @param base_key  Redis 中用户信息键名
   * @param uid       用户 UID
   * @param userinfo  输出参数，用于返回用户信息
   * @return true     成功获取用户信息
   * @return false    数据库中不存在该用户
   */
  bool get_base_info(std::string base_key, int uid,
                     std::shared_ptr<UserInfo> &userinfo);

  /**
   * @brief 通知指定服务器分发文本聊天消息
   *
   * 通过 gRPC 调用目标服务器的 NotifyTextChatMsg 接口，
   * 用于在分布式环境中转发或广播聊天消息。
   * 若 RPC 调用失败，则返回 RPC 调用错误。
   *
   * @param server_ip  目标服务器 IP 地址
   * @param req        文本聊天请求对象
   * @param rtvalue    预留参数（扩展消息内容）
   * @return TextChatMsgRsp 响应结果，包含错误码及消息内容
   */
  TextChatMsgRsp notify_text_chat_msg(std::string server_ip,
                                      const TextChatMsgReq &req,
                                      const Json::Value &rtvalue);
  /**
   * @brief 通过 gRPC 通知指定服务器踢出某用户
   *
   * 该函数会根据 server_ip 从连接池中获取 gRPC 连接，
   * 向目标服务器发送 KickUser 请求并返回响应结果。
   * 若 RPC 调用失败或连接池中不存在对应服务器连接，则返回错误码。
   *
   * @param server_ip  目标服务器的 IP 地址
   * @param req        KickUser 请求对象，包含用户 UID 等信息
   * @return KickUserRsp 响应结果，包含错误码及用户 UID
   */
  KickUserRsp NotifyKickUser(std::string server_ip, const KickUserReq &req);

private:
  ChatGrpcClient();
  /// 连接池集合，一个服务器对应一个连接池，一个连接池有若干RPC连接
  std::unordered_map<server_name_t, std::unique_ptr<ChatServerConnPool>> pools_;
};

#endif // CHAT_RPC_CLIENT_HPP