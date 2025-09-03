#ifndef REDIS_CONN_POOL_HPP
#define REDIS_CONN_POOL_HPP
#include "hiredis.h"
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <queue>
/**
 * @class RedisConnPool
 * @brief 一个基于 hiredis 的 Redis 连接池（线程安全，阻塞获取）。
 * @code
 * RedisConnPool pool(8, "127.0.0.1", 6379, "mypassword");
 * auto* ctx = pool.get_connection();
 * if (!ctx) {
 *       //池已关闭或不可用
 * }
 * // ... 使用 ctx 调用 redisCommand(...)
 * pool.return_connection(ctx); // 使用后务必归还
 * pool.close(); // 关闭池，唤醒等待线程
 * @endcode
 */
class RedisConnPool {
public:
  /**
   * @brief 构造连接池并预建连接。
   * @param pool_size 期望的池大小（上限）
   * @param host Redis 主机地址（如 "127.0.0.1"）
   * @param port Redis 端口（如 6379）
   * @param pwd  Redis 密码（若服务未启用密码，可传空串，但本实现仍会发送 AUTH）
   *
   * @note 构造过程中若 connect 或 AUTH 失败，会跳过该连接，实际可用数量可能小于
   * pool_size。
   * @warning 若传入空密码而服务启用了认证，则 AUTH 会失败，该连接不会放入池中。
   */
  RedisConnPool(size_t pool_size, std::string const& host, int port, std::string const& pwd);

  /**
   * @brief 析构函数。
   *
   */
  ~RedisConnPool();

  /**
   * @brief 获取一个可用的连接。
   * @return 成功时返回 redisContext*；
   *  若池已 close()，或被唤醒时 b_stop_ 为 true，则返回 nullptr。
   *
   * @details 行为：
   * - 若队列为空且未关闭：阻塞等待 cond_，直到有连接归还。
   * - 若在等待期间 close() 被调用：被唤醒并返回 nullptr。
   *
   * @thread_safety 线程安全；可能阻塞。
   */
  redisContext *get_connection();

  /**
   * @brief 归还一个连接到池。
   * @param context 待归还的 redisContext*（必须是从本池 get_connection() 获得）
   *
   * @details 若池已关闭（b_stop_ == true），该连接将被丢弃（不入队），
   * 调用者仍需考虑是否主动 redisFree()。
   *
   * @thread_safety 线程安全；非阻塞。
   */
  void return_connection(redisContext *context);

  /**
   * @brief 关闭连接池并唤醒所有等待线程。
   *
   * @details 调用后：
   * - b_stop_ 置为 true；
   * - 所有 wait() 中的线程被 notify_all() 唤醒，get_connection() 将返回
   * nullptr。
   * - return_connection() 将不再入队新连接。
   */
  void close();

private:
  ///< 池停止标记；一旦为 true，get 将返回 nullptr，return 将丢弃
  std::atomic<bool> b_stop_{false};
  std::size_t pool_size_{0}; ///< 目标池大小上限（可能因失败而实际小于此值）
  std::queue<redisContext *> connections_; ///< 可用连接队列（拥有其所有权）
  std::mutex mutex_;                       ///< 保护队列与停止标记
  std::condition_variable cond_; ///< 连接可用/停止时的条件变量
};
#endif
