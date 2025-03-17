#ifndef RPC_CONNECTION_POOL_HPP
#define RPC_CONNECTION_POOL_HPP
#include "message.grpc.pb.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

class RpcConnectionPool {
public:
  RpcConnectionPool(std::size_t _size, std::string _host, std::string _port);
  ~RpcConnectionPool();

  void close();

  std::unique_ptr<message::VertifyService::Stub> get_connection();

  void return_connection(std::unique_ptr<message::VertifyService::Stub> _connection);

private:
  std::atomic_bool b_stop_;
  std::size_t size_;
  std::string host_;
  std::string port_;

  std::mutex mutex_;
  std::condition_variable cond_;
  std::queue<std::unique_ptr<message::VertifyService::Stub>> connections_;
};

#endif