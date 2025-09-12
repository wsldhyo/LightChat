#include "status_server.hpp"
#include "manager/config_manager.hpp"
#include "manager/redis_manager.hpp"
#include "utility/constant.hpp"
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <climits>
#include "utility/constant.hpp"
std::string generate_unique_string() {
  // 创建UUID对象
  boost::uuids::uuid uuid = boost::uuids::random_generator()();

  // 将UUID转换为字符串
  std::string unique_string = to_string(uuid);
  return unique_string;
}

StatusServer::StatusServer() { init_servers(); }

StatusServer::~StatusServer() {}

Status StatusServer::GetChatServer(ServerContext *context,
                                   const GetChatServerReq *request,
                                   GetChatServerRsp *reply) {
  std::string prefix("status server has received :  ");
  const auto &server = getChatServer();
  reply->set_host(server.host);
  reply->set_port(server.port);
  reply->set_error(static_cast<int32_t>(ErrorCodes::NO_ERROR));
  reply->set_token(generate_unique_string());
  insertToken(request->uid(), reply->token());
  return Status::OK;
}

ChatServer StatusServer::getChatServer() {
  std::lock_guard<std::mutex> guard(server_mtx_);
  auto minServer = servers_.begin()->second;
  auto count_str = RedisMgr::getinstance()->h_get(REDIS_LOGIN_COUNT_PREFIX, minServer.name);
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

    auto count_str =
        RedisMgr::getinstance()->h_get(REDIS_LOGIN_COUNT_PREFIX, server.second.name);
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
  bool success = RedisMgr::getinstance()->get(token_key, token_value);
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

void StatusServer::insertToken(int uid, std::string token) {
  std::string uid_str = std::to_string(uid);
  std::string token_key = std::string(REDIS_USER_TOKEN_PREFIX) + uid_str;
  RedisMgr::getinstance()->set(token_key, token);
}

void StatusServer::init_servers() {
  std::cout << "init servers...\n";
  auto cfg = ConfigManager::getinstance();
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