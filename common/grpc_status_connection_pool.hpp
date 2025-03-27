#ifndef GPRC_STATUS_CONNECTION_POOL
#define GPRC_STATUS_CONNECTION_POOL
#include <string>
#include <atomic>
#include <queue>
#include <condition_variable>
#include "message.grpc.pb.h"

using StatusService = message::StatusService;
 class StatusConPool {
public:
    StatusConPool(std::size_t poolSize, std::string host, std::string port);
    ~StatusConPool() ;
    std::unique_ptr<StatusService::Stub> get_connection() ;
    void return_connection(std::unique_ptr<StatusService::Stub> context) ;
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
