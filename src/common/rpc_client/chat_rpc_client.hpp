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
  std::unordered_map<server_name_t, std::unique_ptr<ChatServerConnPool>> pool_;
};

#endif // CHAT_RPC_CLIENT_HPP