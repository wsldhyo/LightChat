#include "status_server.hpp"
#include "manager/config_manager.hpp"
#include "manager/redis_manager.hpp"
#include "utility/constant.hpp"
#include "utility/defer.hpp"
#include "utility/toolfunc.hpp"
#include <climits>
StatusServer::StatusServer() { init_servers(); }

StatusServer::~StatusServer() {}

Status StatusServer::GetChatServer(ServerContext *context,
                                   const GetChatServerReq *request,
                                   GetChatServerRsp *reply) {
  std::string prefix("status server has received :  ");
  const auto &server = get_chat_server();
  reply->set_host(server.host);
  reply->set_port(server.port);
  reply->set_error(static_cast<int32_t>(ErrorCodes::NO_ERROR));
  // 生成Token，用户密码验证通过后，使用Token作为凭证，后续客户端请求都用 token
  // 证明身份，不必每次带密码
  reply->set_token(generate_unique_string());
  insert_token(request->uid(), reply->token());
  return Status::OK;
}

ChatServer StatusServer::get_chat_server() {
  std::lock_guard<std::mutex> guard(server_mtx_);
  auto minServer = servers_.begin()->second;
  auto lock_key = std::string(REDIS_LOCK_PREFIX);

  // 获取分布式锁
  auto identifier = RedisMgr::get_instance()->acquire_lock(
      lock_key, REDIS_LOCK_TIMEOUT, REDIS_ACQUIRE_TIMEOUT);
  //利用defer解锁
  Defer defer([this, identifier, lock_key]() {
    RedisMgr::get_instance()->release_lock(lock_key, identifier);
  });

  auto count_str =
      RedisMgr::get_instance()->h_get(REDIS_LOGIN_COUNT_PREFIX, minServer.name);
  if (count_str.empty()) {
    //不存在则默认设置为最大
    minServer.con_count = INT_MAX;
  } else {
    minServer.con_count = std::stoi(count_str);
  }

  // 使用范围基于for循环
  for (auto &server : servers_) {

    if (server.second.name == minServer.name) {
      continue;
    }

    auto count_str = RedisMgr::get_instance()->h_get(REDIS_LOGIN_COUNT_PREFIX,
                                                     server.second.name);
    if (count_str.empty()) {
      server.second.con_count = INT_MAX;
    } else {
      server.second.con_count = std::stoi(count_str);
    }

    if (server.second.con_count < minServer.con_count) {
      minServer = server.second;
    }
  }

  return minServer;
}

Status StatusServer::Login(ServerContext *context, const LoginReq *request,
                           LoginRsp *reply) {
  auto uid = request->uid();
  auto token = request->token();

  std::string uid_str = std::to_string(uid);
  std::string token_key = std::string(REDIS_USER_TOKEN_PREFIX) + uid_str;
  std::string token_value = "";
  bool success = RedisMgr::get_instance()->get(token_key, token_value);
  if (!success) {
    std::cout << "uid invalid\n";
    reply->set_error(static_cast<int32_t>(ErrorCodes::UID_INVALID));
    return Status::OK;
  }

  if (token_value != token) {
    std::cout << "token invalid\n";
    reply->set_error(static_cast<int32_t>(ErrorCodes::TOKEN_INVALID));
    return Status::OK;
  }

  reply->set_error(static_cast<int32_t>(ErrorCodes::NO_ERROR));
  reply->set_uid(uid);
  reply->set_token(token);
  return Status::OK;
}

void StatusServer::insert_token(int uid, std::string token) {
  std::string uid_str = std::to_string(uid);
  std::string token_key = std::string(REDIS_USER_TOKEN_PREFIX) + uid_str;
  RedisMgr::get_instance()->set(token_key, token);
}

void StatusServer::init_servers() {
  std::cout << "init servers...\n";
  auto cfg = ConfigManager::get_instance();
  auto server_list = (*cfg)["ChatServers"]["name"];
  std::cout << "ChatServers: " << server_list << '\n';
  std::vector<std::string> words;

  std::stringstream ss(server_list);
  std::string word;

  while (std::getline(ss, word, ',')) {
    words.push_back(word);
  }

  for (auto &word : words) {
    if ((*cfg)[word]["name"].empty()) {
      continue;
    }
    std::cout << "Add server: " << (*cfg)[word]["name"] << '\n';
    ChatServer server;
    server.port = (*cfg)[word]["port"];
    server.host = (*cfg)[word]["host"];
    server.name = (*cfg)[word]["name"];
    servers_[server.name] = server;
  }
}