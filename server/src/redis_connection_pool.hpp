#ifndef REDIS_POOL_HPP
#define REDIS_POOL_HPP
#include <cstddef>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <queue>
class redisContext;
class RedisConnectPool{
public:
    RedisConnectPool(std::size_t _pool_size, char const* _host, int _port, char const* _pwd);
    ~RedisConnectPool();
    redisContext* get_connection();
    void return_connection(redisContext* _connection);
    void close();
private:
    std::size_t pool_size_;
    char const* host_;
    int port_;
    char const* pwd_;
    std::atomic_bool b_stop_;
    std::condition_variable cond_;
    std::mutex mutex_;
    std::queue<redisContext*> connections_;
};

#endif