#include "redis_connection_manager.hpp"
#include "config_manager.hpp"
#include "redis_connection_pool.hpp"
#include <cstring>
#include <iostream>

RedisConnectionManager::RedisConnectionManager() {
  auto &config_manager = *ConfigManager::get_instance();
  auto host = config_manager["Redis"]["host"];
  auto port = config_manager["Redis"]["port"];
  auto pwd = config_manager["Redis"]["pwd"];
  pool_.reset(
      new RedisConnectPool(5, host.c_str(), atoi(port.c_str()), pwd.c_str()));
}
RedisConnectionManager::~RedisConnectionManager() { close(); }

bool RedisConnectionManager::get(const std::string &key, std::string &value) {
  auto connect = pool_->get_connection();
  if (connect == nullptr) {
    return false;
  }
  auto reply =
      static_cast<redisReply *>(redisCommand(connect, "GET %s", key.c_str()));
  // 检查是否获取到replay
  if (reply == nullptr) {
    std::cout << "redis [Get " << key << "] failed" << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  }
  // 检查获取到的类型是否是REDIS的类型
  if (reply->type != REDIS_REPLY_STRING) {
    std::cout << "redis [Get " << key << "] failed" << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  }
  // 操作成功， 设置value
  value = reply->str;
  freeReplyObject(reply);
  pool_->return_connection(connect);
  std::cout << "redis get key:" << value << " success." << std::endl;
  return true;
}
bool RedisConnectionManager::set(const std::string &key,
                                 const std::string &value) {
  // 执行redis命令行
  auto connect = pool_->get_connection();
  if (connect == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "SET %s %s", key.c_str(),
                                          value.c_str());
  // 如果返回nullptr则说明执行失败
  if (reply == nullptr) {
    std::cout << "Execut command [ SET " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  }
  // 如果执行失败则释放连接
  if (!(reply->type == REDIS_REPLY_STATUS &&
        (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0))) {
    std::cout << "Execut command [ SET " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  }
  // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
  freeReplyObject(reply);
  pool_->return_connection(connect);
  std::cout << "Execut command [ SET " << key << "  " << value
            << " ] success ! " << std::endl;
  return true;
}

bool RedisConnectionManager::authorize(const std::string &password) {
  auto connect = pool_->get_connection();
  if (connect == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "AUTH %s", password.c_str());
  if (reply->type == REDIS_REPLY_ERROR) {
    std::cout << "认证失败" << std::endl;
    // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  } else {
    // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(reply);
    pool_->return_connection(connect);
    std::cout << "认证成功" << std::endl;
    return true;
  }
}

bool RedisConnectionManager::l_push(const std::string &key,
                                    const std::string &value) {
  auto connect = pool_->get_connection();
  if (connect == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "LPUSH %s %s", key.c_str(),
                                          value.c_str());
  if (reply == nullptr) {
    std::cout << "Execut command [ LPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  }
  if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
    std::cout << "Execut command [ LPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  }
  std::cout << "Execut command [ LPUSH " << key << "  " << value
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connect);
  return true;
}

bool RedisConnectionManager::l_pop(const std::string &key, std::string &value) {
  auto connect = pool_->get_connection();
  if (connect == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "LPOP %s ", key.c_str());
  if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
    std::cout << "Execut command [ LPOP " << key << " ] failure ! "
              << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  }
  value = reply->str;
  std::cout << "Execut command [ LPOP " << key << " ] success ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connect);
  return true;
}
bool RedisConnectionManager::r_push(const std::string &key,
                                    const std::string &value) {
  auto connect = pool_->get_connection();
  if (connect == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "RPUSH %s %s", key.c_str(),
                                          value.c_str());
  if (NULL == reply) {
    std::cout << "Execut command [ RPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  }
  if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
    std::cout << "Execut command [ RPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  }
  std::cout << "Execut command [ RPUSH " << key << "  " << value
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connect);
  return true;
}
bool RedisConnectionManager::r_pop(const std::string &key, std::string &value) {
  auto connect = pool_->get_connection();
  if (connect == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "RPOP %s ", key.c_str());
  if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
    std::cout << "Execut command [ RPOP " << key << " ] failure ! "
              << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  }
  value = reply->str;
  std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connect);
  return true;
}
bool RedisConnectionManager::h_set(const std::string &key,
                                   const std::string &hkey,
                                   const std::string &value) {
  auto connect = pool_->get_connection();
  if (connect == nullptr) {
    return false;
  }
  auto reply =
      static_cast<redisReply *>(redisCommand(connect, "GET %s", key.c_str()));
  reply = (redisReply *)redisCommand(connect, "HSET %s %s %s", key.c_str(),
                                     hkey.c_str(), value.c_str());
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << value << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  }
  std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connect);
  return true;
}
bool RedisConnectionManager::h_set(const char *key, const char *hkey,
                                   const char *hvalue, size_t hvaluelen) {
  auto connect = pool_->get_connection();
  if (connect == nullptr) {
    return false;
  }
  const char *argv[4];
  size_t argvlen[4];
  argv[0] = "HSET";
  argvlen[0] = 4;
  argv[1] = key;
  argvlen[1] = strlen(key);
  argv[2] = hkey;
  argvlen[2] = strlen(hkey);
  argv[3] = hvalue;
  argvlen[3] = hvaluelen;
  auto reply = (redisReply *)redisCommandArgv(connect, 4, argv, argvlen);
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << hvalue << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  }
  std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connect);
  return true;
}
//bool RedisConnectionManager::h_get(const std::string &key,
//                                   const std::string &hkey,
//                                   std::string &_value) {
//  const char *argv[3];
//  size_t argvlen[3];
//  argv[0] = "HGET";
//  argvlen[0] = 4;
//  argv[1] = key.c_str();
//  argvlen[1] = key.length();
//  argv[2] = hkey.c_str();
//  argvlen[2] = hkey.length();
//  auto connect = pool_->get_connection();
//  if (connect == nullptr) {
//    return false;
//  }
//  auto reply =
//      static_cast<redisReply *>(redisCommand(connect, "GET %s", key.c_str()));
//  reply = (redisReply *)redisCommandArgv(connect, 3, argv, argvlen);
//  if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
//    freeReplyObject(reply);
//    pool_->return_connection(connect);
//    std::cout << "Execut command [ HGet " << key << " " << hkey
//              << "  ] failure ! " << std::endl;
//    return false;
//  }
//  _value = reply->str;
//  freeReplyObject(reply);
//  pool_->return_connection(connect);
//  std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! "
//            << std::endl;
//  return true;
//}
bool RedisConnectionManager::h_get(const std::string &key,
                                   const std::string &hkey,
                                   std::string &_value) {
  // 构建 HGET 命令参数
  const char *argv[] = {"HGET", key.c_str(), hkey.c_str()};
  size_t argvlen[] = {4, key.size(), hkey.size()};

  // 获取连接
  auto connect = pool_->get_connection();
  if (!connect) return false;

  // 发送 HGET 命令
  redisReply *reply = (redisReply *)redisCommandArgv(connect, 3, argv, argvlen);

  // 处理错误
  if (!reply || reply->type == REDIS_REPLY_ERROR) {
    if (reply) freeReplyObject(reply);
    pool_->return_connection(connect);
    std::cerr << "HGET failed for " << key << " " << hkey << std::endl;
    return false;
  }

  // 处理空值
  if (reply->type == REDIS_REPLY_NIL) {
    freeReplyObject(reply);
    pool_->return_connection(connect);
    _value.clear(); // 明确清空输出参数
    return false;
  }

  // 成功获取值
  _value = (reply->type == REDIS_REPLY_STRING) ? reply->str : "";
  freeReplyObject(reply);
  pool_->return_connection(connect);
  return true;
}
bool RedisConnectionManager::del(const std::string &key) {
  auto connect = pool_->get_connection();
  if (connect == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "DEL %s", key.c_str());
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
    std::cout << "Execut command [ Del " << key << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  }
  std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connect);
  return true;
}
bool RedisConnectionManager::exists_key(const std::string &key) {
  auto connect = pool_->get_connection();
  if (connect == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connect, "exists %s", key.c_str());
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER ||
      reply->integer == 0) {
    std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connect);
    return false;
  }
  std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connect);
  return true;
}

void RedisConnectionManager::close() { pool_->close(); }