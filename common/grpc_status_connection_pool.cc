#include "grpc_status_connection_pool.hpp"
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/status.h>
using grpc::Channel;
StatusConPool::StatusConPool(std::size_t _pool_size, std::string host,
                             std::string port)
    : pool_size_(_pool_size), host_(host), port_(port), b_stop_(false) {
  for (std::size_t i = 0; i < pool_size_; ++i) {
    // 根据host和port创建连接
    std::shared_ptr<Channel> channel = grpc::CreateChannel(
        host + ":" + port, grpc::InsecureChannelCredentials());
    // 将连接放入连接池中
    connections_.push(StatusService::NewStub(channel));
  }
}

StatusConPool::~StatusConPool() {
  std::lock_guard<std::mutex> lock(mutex_);
  close();
  while (!connections_.empty()) {
    connections_.pop();
  }
}

std::unique_ptr<StatusService::Stub> StatusConPool::get_connection() {
  std::unique_lock<std::mutex> lock(mutex_);
  // 等待池中有连接可用
  cond_.wait(lock, [this] {
    if (b_stop_) {
      return true;  // 停止连接池时，立即退出
    }
    return !connections_.empty();
  });
  // 如果停止则直接返回空指针
  if (b_stop_) {
    return nullptr;
  }
  // 取出一个连接给外部使用
  auto context = std::move(connections_.front());
  connections_.pop();
  return context;
}

// 归还连接
void StatusConPool::return_connection(std::unique_ptr<StatusService::Stub> context) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (b_stop_) {
    return; // 直接退出，由unique_ptr负责析构连接即可
  }
  connections_.push(std::move(context));
  cond_.notify_one();
}

// 通知所有条件变量，进行停止操作
void StatusConPool::close() {
  b_stop_ = true;
  cond_.notify_all();
}
