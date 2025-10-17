#include "chat_rpc_client.hpp"
#include "manager/config_manager.hpp"
#include "manager/mysql_manager.hpp"
#include "manager/redis_manager.hpp"
#include "pool/chat_server_conn_pool.hpp"
#include "utility/defer.hpp"
#include <json/reader.h>
ChatGrpcClient::ChatGrpcClient() {
  // 读取配置，获取所有对端聊天服务器信息
  // TODO，通过状态服务器获取活跃的聊天服务器，而不是通过硬配置
  auto cfg = ConfigManager::get_instance();
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

  // 确保结束时填充响应中的用户信息
  Defer defer([&rsp, &req]() {
    rsp.set_applyuid(req.applyuid());
    rsp.set_touid(req.touid());
  });

  // 查找目标服务器的连接池
  auto find_iter = pools_.find(server_ip);
  if (find_iter == pools_.end()) {
    return rsp;
  }

  auto &pool = find_iter->second;

  // GRPC 调用
  grpc::ClientContext context;
  auto stub = pool->get_connection();
  grpc::Status status = stub->NotifyAddFriend(&context, req, &rsp);

  // 确保调用结束后归还连接到池
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
  rsp.set_error(static_cast<int32_t>(ErrorCodes::NO_ERROR));

  // 确保函数退出时填充必要的响应信息
  Defer defer([&rsp, &req]() {
    rsp.set_fromuid(req.fromuid());
    rsp.set_touid(req.touid());
  });

  // 查找目标服务器连接池
  auto find_iter = pools_.find(server_ip);
  if (find_iter == pools_.end()) {
    return rsp;
  }

  auto &pool = find_iter->second;
  grpc::ClientContext context;

  // 从连接池获取连接 stub
  auto stub = pool->get_connection();

  // 调用结束后自动归还连接
  Defer defercon([&stub, this, &pool]() {
    pool->return_connection(std::move(stub));
  });

  // 发起 RPC 调用
  grpc::Status status = stub->NotifyAuthFriend(&context, req, &rsp);

  if (!status.ok()) {
    rsp.set_error(static_cast<int32_t>(ErrorCodes::RPC_CALL_FAILED));
    return rsp;
  }

  return rsp;
}

bool ChatGrpcClient::get_base_info(std::string base_key, int uid,
                                   std::shared_ptr<UserInfo> &userinfo) {
  // 尝试从 Redis 获取用户信息
  std::string info_str = "";
  bool b_base = RedisMgr::get_instance()->get(base_key, info_str);
  if (b_base) {
    Json::Reader reader;
    Json::Value root;
    reader.parse(info_str, root);

    // 填充 userinfo
    userinfo->uid = root["uid"].asInt();
    userinfo->name = root["name"].asString();
    userinfo->pwd = root["pwd"].asString();
    userinfo->email = root["email"].asString();
    userinfo->nick = root["nick"].asString();
    userinfo->desc = root["desc"].asString();
    userinfo->sex = root["sex"].asInt();
    userinfo->icon = root["icon"].asString();

    std::cout << "user login uid is  " << userinfo->uid
              << " name is " << userinfo->name
              << " pwd is " << userinfo->pwd
              << " email is " << userinfo->email << std::endl;
  } else {
    // Redis 缓存不存在，从 MySQL 获取
    auto user_info = MysqlMgr::get_instance()->get_user(uid);
    if (user_info == std::nullopt) {
      return false;
    }

    userinfo = std::make_shared<UserInfo>(user_info.value());

    // 写入 Redis 缓存
    Json::Value redis_root;
    redis_root["uid"] = uid;
    redis_root["pwd"] = userinfo->pwd;
    redis_root["name"] = userinfo->name;
    redis_root["email"] = userinfo->email;
    redis_root["nick"] = userinfo->nick;
    redis_root["desc"] = userinfo->desc;
    redis_root["sex"] = userinfo->sex;
    redis_root["icon"] = userinfo->icon;
    RedisMgr::get_instance()->set(base_key, redis_root.toStyledString());
  }
  return true;
}


TextChatMsgRsp
ChatGrpcClient::notify_text_chat_msg(std::string server_ip,
                                     const TextChatMsgReq &req,
                                     const Json::Value &rtvalue) {

  TextChatMsgRsp rsp;
  rsp.set_error(static_cast<int32_t>(ErrorCodes::NO_ERROR));

  // 确保退出时设置响应中的消息数据
  Defer defer([&rsp, &req]() {
    rsp.set_fromuid(req.fromuid());
    rsp.set_touid(req.touid());
    for (const auto &text_data : req.textmsgs()) {
      message::TextChatData *new_msg = rsp.add_textmsgs();
      new_msg->set_msgid(text_data.msgid());
      new_msg->set_msgcontent(text_data.msgcontent());
    }
  });

  // 查找目标服务器连接池
  auto find_iter = pools_.find(server_ip);
  if (find_iter == pools_.end()) {
    return rsp;
  }

  auto &pool = find_iter->second;
  grpc::ClientContext context;

  // 从连接池获取连接 ， 并进行RPC调用
  auto stub = pool->get_connection();

  Defer defercon([&stub, this, &pool]() {
    pool->return_connection(std::move(stub));
  });

  // 发起 RPC 调用
  grpc::Status status = stub->NotifyTextChatMsg(&context, req, &rsp);

  if (!status.ok()) {
    rsp.set_error(static_cast<int32_t>(ErrorCodes::RPC_CALL_FAILED));
    return rsp;
  }

  return rsp;
}

KickUserRsp ChatGrpcClient::NotifyKickUser(std::string server_ip, const KickUserReq& req)
{
    // 构造Grpc响应
    KickUserRsp rsp;
    Defer defer([&rsp, &req]() {
        rsp.set_error(static_cast<int32_t>(ErrorCodes::NO_ERROR));
        rsp.set_uid(req.uid());
    });

    // 查找目标服务器的连接池
    auto find_iter = pools_.find(server_ip);
    if (find_iter == pools_.end()) {
        // 未找到对应连接池，认为踢人成功。 // NOTE 这里也可以返回错误码，让grpc客户端进行处理
        return rsp;
    }

    // 从连接池获取一个 gRPC 连接，进行rpc调用
    auto& pool = find_iter->second;
    grpc::ClientContext context;

    auto stub = pool->get_connection();

    // 确保使用完毕后归还连接
    Defer defercon([&stub, this, &pool]() {
        pool->return_connection(std::move(stub));
    });

    // 调用远程的 NotifyKickUser RPC
    grpc::Status status = stub->NotifyKickUser(&context, req, &rsp);

    // 调用失败则设置错误码
    if (!status.ok()) {
        rsp.set_error(static_cast<int32_t>(ErrorCodes::RPC_CALL_FAILED));
        return rsp;
    }

    return rsp;
}
