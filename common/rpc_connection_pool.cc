#include "rpc_connection_pool.hpp"
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/status.h>

using grpc::Channel;
using message::VertifyService;
RpcConnectionPool::RpcConnectionPool(std::size_t _size, std::string _host,
                                     std::string _port)
    : size_(_size), host_(_host), port_(_port), b_stop_(false) {

  // 初始化rpc连接池
  for (std::size_t i = 0; i < _size; ++i) {
    std::shared_ptr<Channel> channel = grpc::CreateChannel(
        "127.0.0.1:50051", grpc::InsecureChannelCredentials());
    connections_.push(VertifyService::NewStub(channel));
  }
}

RpcConnectionPool::~RpcConnectionPool() {
  std::lock_guard<std::mutex> gaurd(mutex_);
  while (!connections_.empty()) {
    connections_.pop();
  }
}

void RpcConnectionPool::close() {
  std::lock_guard<std::mutex> garud(mutex_);
  cond_.notify_all();
}

std::unique_ptr<VertifyService::Stub> RpcConnectionPool::get_connection() {
  std::unique_lock<std::mutex> gaurd(mutex_);
  cond_.wait(gaurd, [this]() {
    if (b_stop_) {
      return true;
    }
    return !connections_.empty();
  });

  if (b_stop_) {
    return nullptr;
  }

  auto connection = std::move(connections_.front());
  connections_.pop();
  return connection;
}

void RpcConnectionPool::return_connection(
    std::unique_ptr<VertifyService::Stub> _connection) {
  std::lock_guard<std::mutex> garud(mutex_);
  if (b_stop_) {
    return; // 不入池，return后就直接销毁了
  }
  connections_.push(std::move(_connection));
  cond_.notify_one();   // 通知get_connection有连接可用，停止阻塞
}