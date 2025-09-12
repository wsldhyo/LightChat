#ifndef STATUS_SERVER_CONN_POOL_HPP
#define STATUS_SERVER_CONN_POOL_HPP
#include "utility/constant.hpp"
#include "utility/singleton.hpp"
#include "message.grpc.pb.h"
#include "message.pb.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>

using message::StatusService;

class StatusConnPool {
public:
  StatusConnPool(size_t pool_size, std::string host, std::string port);

  ~StatusConnPool();

  std::unique_ptr<StatusService::Stub> get_connection();

  void return_connection(std::unique_ptr<StatusService::Stub> context);

  void close();

private:
  std::atomic<bool> b_stop_;
  size_t pool_size_;
  std::string host_;
  std::string port_;
  std::queue<std::unique_ptr<StatusService::Stub>> connections_;
  std::mutex mutex_;
  std::condition_variable cond_;
};


#endif