#include "chat_rpc_client.hpp"
#include "manager/config_manager.hpp"
#include "pool/chat_server_conn_pool.hpp"
#include "utility/defer.hpp"
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
    pools_[(*cfg)[word]["name"]] = std::make_unique<ChatServerConnPool>(
        5, (*cfg)[word]["host"], (*cfg)[word]["port"]);
  }
}

ChatGrpcClient::~ChatGrpcClient() {}

AddFriendRsp ChatGrpcClient::notify_add_friend(std::string server_ip,
                                               const AddFriendReq &req) {
  AddFriendRsp rsp;
  rsp.set_error(static_cast<int32_t>(ErrorCodes::NO_ERROR));
  Defer defer([&rsp, &req]() {
    // 结束时填充必要信息
    rsp.set_applyuid(req.applyuid());
    rsp.set_touid(req.touid());
  });
  // 取出服务器对应的连接池
  auto find_iter = pools_.find(server_ip);
  if (find_iter == pools_.end()) {
    return rsp;
  }

  auto &pool = find_iter->second;
  // GRPC调用
  grpc::ClientContext context;
  auto stub = pool->get_connection();
  grpc::Status status = stub->NotifyAddFriend(&context, req, &rsp);
  Defer defercon(
      [&stub, this, &pool]() { pool->return_connection(std::move(stub)); });
  if (!status.ok()) {
    rsp.set_error(static_cast<int32_t>(ErrorCodes::RPC_CALL_FAILED));
  }
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
