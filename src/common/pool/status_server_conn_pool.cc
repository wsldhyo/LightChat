#include "status_server_conn_pool.hpp"
using grpc::Channel;

StatusConnPool::StatusConnPool(size_t pool_size, std::string host,
                             std::string port)
    : pool_size_(pool_size), host_(host), port_(port), b_stop_(false) {
  std::string target = host + ":" + port;
  for (size_t i = 0; i < pool_size_; ++i) {
    std::shared_ptr<Channel> channel = grpc::CreateChannel(
        target, grpc::InsecureChannelCredentials());
    connections_.push(StatusService::NewStub(channel));
  }
}

StatusConnPool::~StatusConnPool() {
  std::lock_guard<std::mutex> lock(mutex_);
  close();
  while (!connections_.empty()) {
    connections_.pop();
  }
}

std::unique_ptr<StatusService::Stub> StatusConnPool::get_connection() {
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
  auto context = std::move(connections_.front());
  connections_.pop();
  return context;
}

void StatusConnPool::return_connection(
    std::unique_ptr<StatusService::Stub> context) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (b_stop_) {
    return;
  }
  connections_.push(std::move(context));
  cond_.notify_one();
}

void StatusConnPool::close() {
  b_stop_ = true;
  cond_.notify_all();
}
