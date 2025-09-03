#include "redis_manager.hpp"
#include "config_manager.hpp"
#include "utility/toolfunc.hpp"
#include <cstring>
#include <iostream>
RedisMgr::RedisMgr() {
  auto config_mgr = ConfigManager::getinstance();
  auto host = (*config_mgr)["Redis"]["host"];
  auto port = (*config_mgr)["Redis"]["port"];
  auto pwd = (*config_mgr)["Redis"]["pwd"];
  int port_num{0};
  string_to_int(port, port_num);
  pool_ = std::make_unique<RedisConnPool>(5, host, port_num, pwd);
}

RedisMgr::~RedisMgr() { close(); }

bool RedisMgr::get(std::string const &key, std::string &value) {
  auto connection{pool_->get_connection()};
  if (connection == nullptr) {
    return false;
  }

  auto reply = (redisReply *)redisCommand(connection, "GET %s", key.c_str());
  if (reply == nullptr) {
    std::cout << "[ GET  " << key << " ] failed" << std::endl;
    pool_->return_connection(connection);
    return false;
  }

  if (reply->type != REDIS_REPLY_STRING) {
    std::cout << "[ GET  " << key << " ] failed" << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connection);
    return false;
  }

  value = reply->str;
  freeReplyObject(reply);
  pool_->return_connection(connection);
  std::cout << "Succeed to execute command [ GET " << key << "  ]" << std::endl;
  return true;
}

bool RedisMgr::set(std::string const &key, std::string const &value) {
  auto connection{pool_->get_connection()};
  if (connection == nullptr) {
    return false;
  }

  //执行redis命令行
  auto reply = (redisReply *)redisCommand(connection, "SET %s %s", key.c_str(),
                                          value.c_str());

  //如果返回nullptr则说明执行失败
  if (reply == nullptr) {
    std::cout << "Execut command [ SET " << key << "  " << value
              << " ] failure ! " << std::endl;
    return false;
  }

  //如果执行失败则释放连接
  if (!(reply->type == REDIS_REPLY_STATUS &&
        (strcmp(reply->str, "OK") == 0 || strcmp(reply->str, "ok") == 0))) {
    std::cout << "Execut command [ SET " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connection);
    return false;
  }

  //执行成功 释放redisCommand执行后返回的redisReply所占用的内存
  freeReplyObject(reply);
  pool_->return_connection(connection);
  std::cout << "Execut command [ SET " << key << "  " << value
            << " ] success ! " << std::endl;
  return true;
}

bool RedisMgr::l_push(std::string const &key, std::string const &value) {
  auto connection{pool_->get_connection()};
  if (connection == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connection, "LPUSH %s %s",
                                          key.c_str(), value.c_str());
  if (reply == nullptr) {
    std::cout << "Execut command [ LPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connection);
    return false;
  }

  if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
    std::cout << "Execut command [ LPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connection);
    return false;
  }

  std::cout << "Execut command [ LPUSH " << key << "  " << value
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connection);
  return true;
}

bool RedisMgr::l_pop(std::string const &key, std::string &value) {
  auto connection{pool_->get_connection()};
  if (connection == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connection, "LPOP %s ", key.c_str());
  if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
    std::cout << "Execut command [ LPOP " << key << " ] failure ! "
              << std::endl;
    if (reply) {
      freeReplyObject(reply);
    }
    pool_->return_connection(connection);
    return false;
  }
  value = reply->str;
  std::cout << "Execut command [ LPOP " << key << " ] success ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connection);
  return true;
}
bool RedisMgr::r_push(std::string const &key, std::string const &value) {
  auto connection{pool_->get_connection()};
  if (connection == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connection, "RPUSH %s %s",
                                          key.c_str(), value.c_str());
  if (reply == nullptr) {
    std::cout << "Execut command [ RPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    pool_->return_connection(connection);
    return false;
  }

  if (reply->type != REDIS_REPLY_INTEGER || reply->integer <= 0) {
    std::cout << "Execut command [ RPUSH " << key << "  " << value
              << " ] failure ! " << std::endl;
    freeReplyObject(reply);
    pool_->return_connection(connection);
    return false;
  }

  std::cout << "Execut command [ RPUSH " << key << "  " << value
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connection);
  return true;
}

bool RedisMgr::r_pop(std::string const &key, std::string &value) {
  auto connection{pool_->get_connection()};
  if (connection == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connection, "RPOP %s ", key.c_str());
  if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
    std::cout << "Execut command [ RPOP " << key << " ] failure ! "
              << std::endl;
    if (reply) {
      freeReplyObject(reply);
    }
    pool_->return_connection(connection);
    return false;
  }
  value = reply->str;
  std::cout << "Execut command [ RPOP " << key << " ] success ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connection);
  return true;
}

bool RedisMgr::h_set(std::string const &key, std::string const &hkey,
                     std::string const &value) {
  auto connection{pool_->get_connection()};
  if (connection == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(
      connection, "HSET %s %s %s", key.c_str(), hkey.c_str(), value.c_str());
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << value << " ] failure ! " << std::endl;
    if (reply) {
      freeReplyObject(reply);
    }
    pool_->return_connection(connection);
    return false;
  }
  std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << value
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connection);
  return true;
}

bool RedisMgr::h_set(char const *key, char const *hkey, char const *hvalue,
                     size_t hvaluelen) {
  auto connection{pool_->get_connection()};
  if (connection == nullptr) {
    return false;
  }

  char const *argv[4];
  size_t argvlen[4];
  argv[0] = "HSET";
  argvlen[0] = 4;
  argv[1] = key;
  argvlen[1] = strlen(key);
  argv[2] = hkey;
  argvlen[2] = strlen(hkey);
  argv[3] = hvalue;
  argvlen[3] = hvaluelen;

  auto reply = (redisReply *)redisCommandArgv(connection, 4, argv, argvlen);
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
    std::cout << "Execut command [ HSet " << key << "  " << hkey << "  "
              << hvalue << " ] failure ! " << std::endl;
    if (reply) {
      freeReplyObject(reply);
    }
    pool_->return_connection(connection);
    return false;
  }
  std::cout << "Execut command [ HSet " << key << "  " << hkey << "  " << hvalue
            << " ] success ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connection);
  return true;
}

std::string RedisMgr::h_get(std::string const &key, std::string const &hkey) {
  auto connection{pool_->get_connection()};
  if (connection == nullptr) {
    return "";
  }

  char const *argv[3];
  size_t argvlen[3];
  argv[0] = "HGET";
  argvlen[0] = 4;
  argv[1] = key.c_str();
  argvlen[1] = key.length();
  argv[2] = hkey.c_str();
  argvlen[2] = hkey.length();

  auto reply = (redisReply *)redisCommandArgv(connection, 3, argv, argvlen);
  if (reply == nullptr || reply->type == REDIS_REPLY_NIL) {
    std::cout << "Execut command [ HGet " << key << " " << hkey
              << "  ] failure ! " << std::endl;
    if (reply) {
      freeReplyObject(reply);
    }
    pool_->return_connection(connection);
    return "";
  }

  std::string value = reply->str;
  freeReplyObject(reply);
  pool_->return_connection(connection);
  std::cout << "Execut command [ HGet " << key << " " << hkey << " ] success ! "
            << std::endl;
  return value;
}

bool RedisMgr::del(std::string const &key) {
  auto connection{pool_->get_connection()};
  if (connection == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connection, "DEL %s", key.c_str());
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER) {
    std::cout << "Execut command [ Del " << key << " ] failure ! " << std::endl;
    if (reply) {
      freeReplyObject(reply);
    }
    pool_->return_connection(connection);
    return false;
  }
  std::cout << "Execut command [ Del " << key << " ] success ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connection);
  return true;
}

bool RedisMgr::existskey(std::string const &key) {
  auto connection{pool_->get_connection()};
  if (connection == nullptr) {
    return false;
  }
  auto reply = (redisReply *)redisCommand(connection, "exists %s", key.c_str());
  if (reply == nullptr || reply->type != REDIS_REPLY_INTEGER ||
      reply->integer == 0) {
    std::cout << "Not Found [ Key " << key << " ]  ! " << std::endl;
    if (reply) {
      freeReplyObject(reply);
    }
    pool_->return_connection(connection);
    return false;
  }
  std::cout << " Found [ Key " << key << " ] exists ! " << std::endl;
  freeReplyObject(reply);
  pool_->return_connection(connection);
  return true;
}

void RedisMgr::close() { pool_->close(); }
