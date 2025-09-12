#ifndef CHAT_SERVER_CONN_POOL_HPP
#define CHAT_SERVER_CONN_POOL_HPP
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>

// TODO
// 各个RPC连接池，代码大体一致，可以抽象出一个基本连接池类，用组合模式简化代码
using message::ChatService;
class ChatServerConnPool {
public:
  ChatServerConnPool(std::size_t size, std::string host, std::string port);

  ~ChatServerConnPool();

  std::unique_ptr<ChatService::Stub> get_connection();

  void return_connection(std::unique_ptr<ChatService::Stub> context);

  void close();

private:
  std::atomic_bool b_stop_;
  std::size_t size_;
  std::string host_; /// 对端地址
  std::string port_; /// 对端端口
  std::queue<std::unique_ptr<ChatService::Stub>> connections_;

  std::mutex mutex_;
  std::condition_variable cond_;
};

#endif // CHAT_SERVER_CONN_POOL_HPP
