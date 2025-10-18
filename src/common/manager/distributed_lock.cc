#include "distributed_lock.hpp"
#include "utility/toolfunc.hpp"
#include <chrono>
#include <cstdlib>
#include <hiredis.h>
#include <iostream>
#include <string>
#include <thread>
DistLock &DistLock::get_instance() {
  static DistLock lock;
  return lock;
}

DistLock::~DistLock() {}

std::string DistLock::acquire(redisContext *context,
                              const std::string &lock_name, int lockTimeout,
                              int acquireTimeout) {
  // 生成唯一标识符，用于区分不同客户端的锁
  std::string identifier = generate_unique_string();
  std::string lockKey = "lock:" + lock_name;

  // 计算锁获取操作的超时截止时间
  auto endTime =
      std::chrono::steady_clock::now() + std::chrono::seconds(acquireTimeout);

  while (std::chrono::steady_clock::now() < endTime) {
    // 使用 SET NX EX 尝试设置锁（仅在 key 不存在时设置，并带过期时间）
    redisReply *reply = (redisReply *)redisCommand(
        context, "SET %s %s NX EX %d", lockKey.c_str(), identifier.c_str(),
        lockTimeout);

    if (reply != nullptr) { // Redis成功执行命令
      if (reply->type == REDIS_REPLY_STATUS &&
          std::string(reply->str) == "OK") { // 命令执行后，成功获取到锁
        freeReplyObject(reply);
        return identifier; // 获取成功，返回锁标识符
      }
      freeReplyObject(reply);
    }

    // 未获取到锁，短暂休眠 1ms 再重试，避免 CPU 忙等
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // 超时仍未获取锁，返回空字符串
  return "";
}

bool DistLock::release(redisContext *context, const std::string &lock_name,
                       const std::string &identifier) {
  std::string lockKey = "lock:" + lock_name;

  // Lua 脚本：仅当锁的值与当前标识符匹配时才删除 key
  const char *luaScript = "if redis.call('get', KEYS[1]) == ARGV[1] then "
                          "   return redis.call('del', KEYS[1]) "
                          "else "
                          "   return 0 "
                          "end";

  // 通过 EVAL 执行 Lua 脚本：1 表示传入 1 个 key
  redisReply *reply =
      (redisReply *)redisCommand(context, "EVAL %s 1 %s %s", luaScript,
                                 lockKey.c_str(), identifier.c_str());

  bool success = false;
  if (reply != nullptr) {
    // 若返回整数值 1，表示删除成功（锁已释放）
    if (reply->type == REDIS_REPLY_INTEGER && reply->integer == 1) {
      success = true;
    }
    freeReplyObject(reply);
  }

  return success;
}