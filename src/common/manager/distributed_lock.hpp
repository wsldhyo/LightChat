#ifndef DISTRIBUTED_LOCK_HPP
#define DISTRIBUTED_LOCK_HPP
#include <string>
class redisContext;

/**
 * @class DistLock
 * @brief 基于 Redis 实现的分布式锁管理类。
 *
 * 该类提供分布式环境下的互斥访问控制，确保多个进程或服务实例
 * 在共享资源访问时不会发生并发冲突。
 *
 * - **加锁**：使用 Redis 命令
 *   `SET key value NX EX timeout`
            key	锁的名字，例如 lock:inventory
            value	锁的持有者标识（UUID）
            NX	只有当 key 不存在时才设置（防止重复上锁）
            EX timeout	让锁自动过期（防止死锁）
 *   保证同一时间只有一个客户端成功持有锁；
 *   通过 `EX` 参数设置自动过期，防止死锁。
 *
 * - **解锁**：使用 Lua 脚本判断锁的持有者是否一致，
 *   仅当 `value` 匹配时才删除对应 key，防止误删他人锁。
 *
 * ### 使用方法：
 * auto& lock = DistLock::Inst();
 * std::string id = lock.acquireLock(ctx, "task_key", 5, 10);
 * if (!id.empty()) {
 *     // 执行业务逻辑
 *     lock.releaseLock(ctx, "task_key", id);
 * }
 * ```
 *
 * @note 本类为单例设计，通过 get_instance() 获取全局实例。
 */
class DistLock {
public:
  static DistLock &get_instance();
  ~DistLock();
  /**
   * @brief 尝试从 Redis 获取一个分布式锁。
   *
   * 该函数使用 Redis 的 `SET key value NX EX timeout` 命令实现分布式锁。
   * 若在指定的超时时间内成功获取锁，则返回该锁的唯一标识符（UUID）；
   * 若超时仍未能获取锁，则返回空字符串。
   *
   * @param context        Redis 连接上下文。
   * @param lockName       锁的名称（逻辑标识）。
   * @param lockTimeout    锁的过期时间（秒），防止死锁。
   * @param acquireTimeout 获取锁的最大等待时间（秒）。
   * @return std::string   成功时返回唯一标识符（UUID），失败时返回空字符串。
   */
  std::string acquire(redisContext *context, const std::string &lockName,
                      int lockTimeout, int acquireTimeout);

  /**
   * @brief 释放分布式锁，仅限锁的持有者调用。
   *
   * 该函数通过执行 Lua 脚本，确保只有持有相同标识符（identifier）的客户端
   * 才能删除对应的锁，避免误删其他客户端的锁。
   *
   * @param context      Redis 连接上下文。
   * @param lockName     锁的名称（逻辑标识）。
   * @param identifier   获取锁时返回的唯一标识符。
   * @return bool        若成功释放锁返回 true，否则返回 false。
   */
  bool release(redisContext *context, const std::string &lockName,
               const std::string &identifier);


private:
  DistLock() = default;
};
#endif // DISTRIBUTED_LOCK_HPP