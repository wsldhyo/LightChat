#include "status_server_impl.hpp"
#include "../common/config_manager.hpp"
#include "../common/constant.hpp"
#include "../common/redis_connection_manager.hpp"
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <string>

#include <climits>
std::string generate_unique_string() {
  // 创建UUID对象
  boost::uuids::uuid uuid = boost::uuids::random_generator()();

  // 将UUID转换为字符串
  std::string unique_string = boost::uuids::to_string(uuid);

  return unique_string;
}

Status StatusServiceImpl::GetChatServer(ServerContext *context,
                                        const GetChatServerReq *request,
                                        GetChatServerRsp *reply) {
  std::string prefix("status server has received :  ");
  const auto &server = get_chat_server();
  reply->set_host(server.host);
  reply->set_port(server.port);
  reply->set_error(static_cast<int>(ErrorCode::NO_ERROR));
  reply->set_token(generate_unique_string());
  insert_token(request->uid(), reply->token());
  return Status::OK;
}

StatusServiceImpl::StatusServiceImpl() {
  auto sp_cfg = ConfigManager::get_instance();
  auto &cfg = *sp_cfg;

  auto server_list = cfg["ChatServers"]["name"];

  std::vector<std::string> words;

  std::stringstream ss(server_list);
  std::string word;
  std::cout << "servers:" << ss.str() << std::endl;
  while (std::getline(ss, word, ',')) {
    std::cout << "word: " <<  word << std::endl;
    words.push_back(word);
  }

  for (auto &word : words) {
    if (cfg[word]["name"].empty()) {
      continue;
    }

    ChatServer server;
    server.port = cfg[word]["port"];
    server.host = cfg[word]["host"];
    server.name = cfg[word]["name"];
    _servers[server.name] = server;
    std::cout << "add server: " << server.name << std::endl;
  }
}

ChatServer StatusServiceImpl::get_chat_server() {
  std::lock_guard<std::mutex> guard(_server_mtx);
  auto minServer = _servers.begin()->second;
  std::string count_str;
  RedisConnectionManager::get_instance()->h_get(LOGIN_COUNT, minServer.name,
                                                count_str);
  if (count_str.empty()) {
    // 不存在则默认设置为最大
    minServer.con_count = INT_MAX;
  } else {
    minServer.con_count = std::stoi(count_str);
  }

  // 寻找服务数最小（登录数最小）的服务器，进行负载均衡
  for (auto &server : _servers) {

    if (server.second.name == minServer.name) {
      continue;
    }

    RedisConnectionManager::get_instance()->h_get(
        LOGIN_COUNT, server.second.name, count_str);
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

Status StatusServiceImpl::Login(ServerContext *context, const LoginReq *request,
                                LoginRsp *reply) {
  auto uid = request->uid();
  auto token = request->token();

  std::string uid_str = std::to_string(uid);
  std::string token_key = USERTOKENPREFIX + uid_str;
  std::string token_value = "";
  bool success =
      RedisConnectionManager::get_instance()->get(token_key, token_value);
  if (!success) {
    reply->set_error(static_cast<int>(ErrorCode::UID_VALID));
    return Status::OK;
  }

  if (token_value != token) {
    reply->set_error(static_cast<int>(ErrorCode::TOKEN_VALID));
    return Status::OK;
  }
  reply->set_error(static_cast<int>(ErrorCode::NO_ERROR));
  reply->set_uid(uid);
  reply->set_token(token);
  return Status::OK;
}

void StatusServiceImpl::insert_token(int uid, std::string token) {
  std::string uid_str = std::to_string(uid);
  std::string token_key = USERTOKENPREFIX + uid_str;
  RedisConnectionManager::get_instance()->set(token_key, token);
}