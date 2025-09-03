#include "redis_conn_pool.hpp"
#include <iostream>

RedisConnPool::RedisConnPool(size_t pool_size, std::string const &host,
                             int port, std::string const &pwd)
    : pool_size_(pool_size), b_stop_(false) {

  for (size_t i = 0; i < pool_size_; ++i) {
    auto *context = redisConnect(host.c_str(), port);
    if (context == nullptr || context->err != 0) {
      if (context)
        redisFree(context);
      continue;
    }

    auto reply = (redisReply *)redisCommand(context, "AUTH %s", pwd.c_str());
    if (!reply) {
      std::cout << "AUTH command failed: " << context->errstr << std::endl;
      redisFree(context);
      continue;
    }

    if (reply->type == REDIS_REPLY_ERROR) {
      std::cout << "AUTH failed: " << reply->str << std::endl;
      freeReplyObject(reply);
      redisFree(context);
      continue;
    }

    // AUTH 成功
    freeReplyObject(reply);
    connections_.push(context);
    std::cout << "AUTH success" << std::endl;
  }
}

RedisConnPool::~RedisConnPool() {
  std::lock_guard<std::mutex> lock(mutex_);
  while (!connections_.empty()) {
    redisFree(connections_.front());
    connections_.pop();
  }
}

redisContext *RedisConnPool::get_connection() {
  // 阻塞等待至队列非空
  std::unique_lock<std::mutex> lock(mutex_);
  cond_.wait(lock, [this] {
    if (b_stop_) {
      return true;
    }
    return !connections_.empty();
  });
  //如果停止则直接返回空指针
  if (b_stop_) {
    return nullptr;
  }
  auto *context = connections_.front();
  connections_.pop();
  return context;
}

void RedisConnPool::return_connection(redisContext *context) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (b_stop_) {
    // 池已经关闭，则直接释放该连接
    redisFree(context);
    return;
  }
  connections_.push(context);
  cond_.notify_one();
}

void RedisConnPool::close() {
  //关闭连接池，并唤醒所有等待线程，使其从 wait 返回并得到 nullptr
  b_stop_ = true;
  cond_.notify_all();
}
