#ifndef REDIS_MANAGER_HPP
#define REDIS_MANAGER_HPP
#include "pool/redis_conn_pool.hpp"
#include "utility/singleton.hpp"

/**
 * @class RedisMgr
 * @brief 封装 Redis 客户端操作，提供常用 Redis 命令的 C++ 接口
 *
 * 功能：
 * - 支持单例模式访问
 * - 提供 Redis 连接管理、认证和关闭接口
 * - 提供常用 Redis 命令封装：GET/SET、列表操作、哈希操作、删除、判断键存在
 *
 * 使用方式：
 * @code
 * auto redis = RedisMgr::get_instance();
 * redis->connect("127.0.0.1", 6379);
 * redis->set("key", "value");
 * std::string val;
 * redis->get("key", val);
 * @endcode
 */
class RedisMgr : public Singleton<RedisMgr>,
                 public std::enable_shared_from_this<RedisMgr> {
  friend class Singleton<RedisMgr>;

public:
  /**
   * @brief 析构函数，释放 Redis 连接
   */
  ~RedisMgr();

  /**
   * @brief 获取指定键的值
   * @param key 待获取的键
   * @param value 输出参数，返回对应键zM值
   * @return true 获取成功，false 获取失败
   */
  bool get(std::string const &key, std::string &value);

  /**
   * @brief 设置指定键的值
   * @param key 待设置的键
   * @param value 待设置的值
   * @return true 设置成功，false 设置失败
   */
  bool set(std::string const &key, const std::string &value);

  /**
   * @brief 插入Redis队列左边（队头）
   * @param key 列表键
   * @param value 待插入的值
   * @return true 成功，false 失败
   */
  bool l_push(std::string const &key, const std::string &value);

  /**
   * @brief 弹出Redis队头的元素
   * @param key 列表键
   * @param value 输出参数，返回弹出的值
   * @return true 成功，false 失败
   */
  bool l_pop(std::string const &key, std::string &value);

  /**
   * @brief 将值插入队尾
   * @param key 列表键
   * @param value 待插入的值
   * @return true 成功，false 失败
   */
  bool r_push(std::string const &key, const std::string &value);

  /**
   * @brief 弹出列表队尾元素
   * @param key 列表键
   * @param value 输出参数，返回弹出的值
   * @return true 成功，false 失败
   */
  bool r_pop(std::string const &key, std::string &value);

  /**
   * @brief 设置哈希表字段值，哈希操作函数
   * @param key 哈希表键，外层 key，即哈希表的名字
   * @param hkey 哈希表中字段，内层key
   * @param value 待设置的值
   * @return true 成功，false 失败
   */
  bool h_set(std::string_view key, std::string const &hkey,
             std::string const &value);

  /**
   * @brief 设置哈希表字段值（支持二进制数据），哈希操作函数
   * @param key 哈希表键，外层 key，即哈希表的名字
   * @param hkey 哈希表中字段，内层key
   * @param hvalue 字段值
   * @param hvaluelen 字段值长度
   * @return true 成功，false 失败
   */
  bool h_set(char const *key, char const *hkey, char const *hvalue,
             size_t hvaluelen);

  /**
   * @brief 获取哈希表字段值，哈希操作函数
   * @param key 哈希表键，外层 key，即哈希表的名字
   * @param hkey 哈希表中字段，内层key
   * @return 字段对应的值，如果不存在返回空字符串
   */
  std::string h_get(std::string_view key, std::string const &hkey);

  /**
   * @brief 删除指定键
   * @param key 待删除的键
   * @return true 成功，false 失败
   */
  bool del(std::string const &key);

  bool h_del(std::string_view key, std::string const &field);

  /**
   * @brief 判断指定键是否存在
   * @param key 待检测的键
   * @return true 存在，false 不存在
   */
  bool existskey(std::string const &key);

  /**
   * @brief 关闭 Redis 连接
   */
  void close();

  std::string acquire_lock(const std::string &lock_name, int lock_timeout,
                           int acquire_timeout);

  bool release_lock(const std::string &lock_name,
                    const std::string &identifier);

  // 增加登录计数
  void increase_count(std::string const &server_name);

  // 减少登录计数
  void decrease_count(std::string const &server_name);

  // 初始化登录计数
  void init_count(std::string const &server_name);

  // 删除某服务器的登录计数缓存
  void del_count(std::string const &server_name);

private:
  RedisMgr();

  std::unique_ptr<RedisConnPool> pool_;
};
#endif