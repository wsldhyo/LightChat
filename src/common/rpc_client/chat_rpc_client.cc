#include "chat_rpc_client.hpp"
#include "manager/config_manager.hpp"
#include "pool/chat_server_conn_pool.hpp"
ChatGrpcClient::ChatGrpcClient() {
  // 读取配置，获取所有对端聊天服务器信息
  // TODO，通过状态服务器获取活跃的聊天服务器，而不是通过硬配置
  auto cfg = ConfigManager::getinstance();
  auto server_list = (*cfg)["PeerServer"]["servers"];
  std::vector<std::string> words;
  std::stringstream ss(server_list);
  std::string word;
  while (std::getline(ss, word, ',')) {
    words.push_back(word);
  }

  // 初始化聊天服务器连接池
  for (auto &word : words) {
    if ((*cfg)[word]["name"].empty()) {
      continue;
    }
    pool_[(*cfg)[word]["name"]] = std::make_unique<ChatServerConnPool>(
        5, (*cfg)[word]["host"], (*cfg)[word]["port"]);
  }
}

ChatGrpcClient::~ChatGrpcClient() {}

AddFriendRsp ChatGrpcClient::notify_add_friend(std::string server_ip,
                                               const AddFriendReq &req) {
  AddFriendRsp rsp;
  return rsp;
}

AuthFriendRsp ChatGrpcClient::notify_auth_friend(std::string server_ip,
                                                 const AuthFriendReq &req) {
  AuthFriendRsp rsp;
  return rsp;
}

bool ChatGrpcClient::get_base_info(std::string base_key, int uid,
                                   std::shared_ptr<UserInfo> &userinfo) {
  return true;
}

TextChatMsgRsp
ChatGrpcClient::notify_text_chat_msg(std::string server_ip,
                                     const TextChatMsgReq &req,
                                     const Json::Value &rtvalue) {

  TextChatMsgRsp rsp;
  return rsp;
}
