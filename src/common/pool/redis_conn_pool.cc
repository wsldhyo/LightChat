#include "redis_conn_pool.hpp"
#include <iostream>

RedisConnPool::RedisConnPool(size_t pool_size, std::string const &host,
                             int port, std::string const &pwd)
    : pool_size_(pool_size), b_stop_(false), host_(host), port_(port),
      pwd_(pwd), check_count_(0), fail_count_(0) {

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
    std::cout << "AUTH success\n";
  }

  check_thread_ = std::thread([this]() {
    while (!b_stop_) {
      ++check_count_;
      if (check_count_ > 30) {
        check_connection();
        check_count_ = 0;
      }
      std::this_thread::sleep_for(
          std::chrono::seconds(1)); // 每间隔30s发送一次Redis命令
    }
  });
}

RedisConnPool::~RedisConnPool() {
  // 先等待后台线程结束，并阻止后续对Redis的操作（阻止取出连接）
  {
    std::lock_guard<std::mutex> lock(mutex_);
    close();
    cond_.notify_all();
  }

  if (check_thread_.joinable()) {
    check_thread_.join();
  }

  // 清理资源
  {
    std::lock_guard<std::mutex> lock(mutex_);
    while (!connections_.empty()) {
      redisFree(connections_.front());
      connections_.pop();
    }
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

redisContext *RedisConnPool::get_connection_nonblock() {
  std::lock_guard<std::mutex> lock(mutex_);
  if (b_stop_) {
    return nullptr;
  }

  if (connections_.empty()) {
    return nullptr;
  }

  auto context = connections_.front();
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

void RedisConnPool::check_connection() {
  size_t pool_size;
  {
    // 先获取连接数
    std::lock_guard<std::mutex> lock(mutex_);
    pool_size = connections_.size();
  }

  for (int i = 0; i < pool_size && !b_stop_; ++i) {
    redisContext *ctx = nullptr;
    // 取出一个连接并检查
    bool bsuccess = false;
    auto *context = get_connection_nonblock();
    if (context == nullptr) {
      break;
    }

    redisReply *reply = nullptr;
    try {
      // 对Redis执行一次PING命令，防止Redis断开连接
      reply = (redisReply *)redisCommand(context, "PING");
      // 检查底层Redis连接是否有错误
      if (context->err) {
        std::cout << "Connection error: " << context->err << std::endl;
        if (reply) {
          freeReplyObject(reply);
        }
        redisFree(context);
        fail_count_++;
        continue;
      }

      // 检查本次操作的结果
      if (!reply || reply->type == REDIS_REPLY_ERROR) {
        std::cout << "reply is null, redis ping failed: " << std::endl;
        if (reply) {
          freeReplyObject(reply);
        }
        redisFree(context);
        fail_count_++;
        continue;
      }
      // 都没有问题，则将连接归还池
      // std::cout << "connection alive" << std::endl;
      freeReplyObject(reply);
      return_connection(context);
    } catch (std::exception &exp) {
      if (reply) {
        freeReplyObject(reply);
      }

      redisFree(context);
      fail_count_++;
    }
  }

  // 尝试重连，恢复池中连接数量
  bool need_reconnect{fail_count_ > 0};
  {
    std::lock_guard<std::mutex> lock(mutex_);
    need_reconnect = connections_.empty() && !b_stop_;
  }
  while (need_reconnect) {
    auto res = reconnect();
    if (res) {
      fail_count_--;
    } else {
      break;
    }
  }
}

bool RedisConnPool::reconnect() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (connections_.size() >= 10) {
      std::cout << "redis connection resume!\n";
      fail_count_ = 0;
      return true;
    }
  }
  // 尝试创建新连接
  auto context = redisConnect(host_.c_str(), port_);
  if (context == nullptr || context->err != 0) {
    if (context != nullptr) {
      std::cout << "redis reconnection error.error code:" << context->err
                << '\n';
      redisFree(context);
    }
    return false;
  }

  // 进行认证操作
  auto reply = (redisReply *)redisCommand(context, "AUTH %s", pwd_.c_str());
  if (reply->type == REDIS_REPLY_ERROR) {
    std::cout << "redis reconnect auth failed";
    freeReplyObject(reply);
    redisFree(context);
    return false;
  }

  freeReplyObject(reply);
  std::cout << "redis reconnect success!\n";
  return_connection(context);
  return true;
}