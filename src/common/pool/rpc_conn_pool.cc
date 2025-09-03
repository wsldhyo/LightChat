#include "rpc_conn_pool.hpp"

RPCConnPool::RPCConnPool(size_t pool_size, std::string host, std::string port)
    : pool_size_(pool_size), host_(host), port_(port), b_stop_(false) {
  // 预初始化多个Stub到池中
  for (size_t i = 0; i < pool_size_; ++i) {
    std::shared_ptr<Channel> channel = grpc::CreateChannel(
        host + ":" + port, grpc::InsecureChannelCredentials());

    connections_.push(VertifyService::NewStub(channel));
  }
}

RPCConnPool::~RPCConnPool() {
  std::lock_guard<std::mutex> lock(mutex_);
  // 停止池API的阻塞
  close();
  // 销毁池中的连接
  while (!connections_.empty()) {
    connections_.pop();
  }
}

std::unique_ptr<VertifyService::Stub> RPCConnPool::get_connection() {
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

void RPCConnPool::return_connection(
  std::unique_ptr<VertifyService::Stub> context) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (b_stop_) {
    return;
  }
  connections_.push(std::move(context));
  cond_.notify_one();
}

void RPCConnPool::close() {
  b_stop_ = true;
  cond_.notify_all();
}
