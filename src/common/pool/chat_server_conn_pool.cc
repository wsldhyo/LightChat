#include "chat_server_conn_pool.hpp"
#include <grpcpp/grpcpp.h>

using grpc::Channel;
using message::ChatService;

ChatServerConnPool::ChatServerConnPool(std::size_t size, std::string host,
                                       std::string port)
    : size_(size), host_(host), port_(port), b_stop_(false) {
  auto target_addr = host_ + ":" + port;
  // 预先创建多个rpc连接
  std::shared_ptr<Channel> channel =
      grpc::CreateChannel(target_addr, grpc::InsecureChannelCredentials());
  connections_.push(ChatService::NewStub(channel));
}

ChatServerConnPool::~ChatServerConnPool() {
  std::lock_guard<std::mutex> lock(mutex_);
  close();
}

std::unique_ptr<ChatService::Stub> ChatServerConnPool::get_connection() {
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

void ChatServerConnPool::return_connection(
    std::unique_ptr<ChatService::Stub> context) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (b_stop_) {
    return;
  }
  connections_.push(std::move(context));
  cond_.notify_one();
}

void ChatServerConnPool::close() {
  b_stop_ = true;
  cond_.notify_one();
}