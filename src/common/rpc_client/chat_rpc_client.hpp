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
  AuthFriendRsp notify_auth_friend(std::string server_ip,
                                   const AuthFriendReq &req);
  bool get_base_info(std::string base_key, int uid,
                     std::shared_ptr<UserInfo> &userinfo);
  TextChatMsgRsp notify_text_chat_msg(std::string server_ip,
                                      const TextChatMsgReq &req,
                                      const Json::Value &rtvalue);

private:
  ChatGrpcClient();
  /// 连接池集合，一个服务器对应一个连接池，一个连接池有若干RPC连接
  std::unordered_map<server_name_t, std::unique_ptr<ChatServerConnPool>> pools_;
};

#endif // CHAT_RPC_CLIENT_HPP