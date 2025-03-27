#include "redis_connection_pool.hpp"
#include <hiredis.h>
#include <iostream>
RedisConnectPool::RedisConnectPool(size_t poolSize, const char *host, int port,
                                   char const* _pwd)
    : pool_size_(poolSize), host_(host), port_(port), b_stop_(false) {

  // 通过redisConnect获取一定数量的redis连接
  for (size_t i = 0; i < pool_size_; ++i) {
    auto *context = redisConnect(host, port);
    if (context == nullptr) {
      continue;
    }

    // 删错错误连接
    if (context->err != 0) {
      redisFree(context);
      continue;
    }

    // 进行redis认证
    auto reply = (redisReply *)redisCommand(context, "AUTH %s", _pwd);
    if (reply->type == REDIS_REPLY_ERROR) {
      std::cout << "认证失败" << std::endl;
      // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
      freeReplyObject(reply);
      continue;
    }
    // 执行成功 释放redisCommand执行后返回的redisReply所占用的内存
    freeReplyObject(reply);
    std::cout << "认证成功" << std::endl;
    connections_.push(context);
  }
}

RedisConnectPool::~RedisConnectPool() {
  std::lock_guard<std::mutex> lock(mutex_);
  while (!connections_.empty()) {
    connections_.pop();
  }
}

redisContext *RedisConnectPool::get_connection() {
  redisContext *context = nullptr;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait(lock, [this] {
      if (b_stop_) {
        return true;
      }
      return !connections_.empty();
    });
    context = connections_.front();
    connections_.pop();
  }
  // 如果停止则直接返回空指针
  if (b_stop_) {
    return nullptr;
  }
  return context;
}

void RedisConnectPool::return_connection(redisContext *_connection) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (b_stop_) {
    return;
  }
  connections_.push(_connection);
  cond_.notify_one();
}


    void RedisConnectPool::close(){
        b_stop_ = true;
        cond_.notify_all();  // 唤醒所有线程，执行退出操作
    }