#ifndef MYSQL_CONN_POOL_HPP
#define MYSQL_CONN_POOL_HPP
#include <condition_variable>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <memory>
#include <mutex>
#include <queue>
#include <string>

struct SqlConnection {
  SqlConnection(sql::Connection *con,
                std::chrono::steady_clock::time_point lasttime)
      : con_(con), last_operate_time_(lasttime) {}
  std::unique_ptr<sql::Connection> con_;
  std::chrono::steady_clock::time_point last_operate_time_;
};
using sql_conn_uptr_t = std::unique_ptr<SqlConnection>;
using sql_driver_t = sql::mysql::MySQL_Driver;

/**
 * @brief MySQL 连接池类
 *
 * 该类实现了一个简单的 MySQL 连接池，用于管理数据库连接的复用，
 * 避免频繁创建和销毁连接带来的开销。
 */
class MysqlConnPool {
public:
  /**
   * @brief 构造函数，初始化连接池
   *
   * @param url 数据库连接地址（如 "tcp://127.0.0.1:3306"）
   * @param user 数据库用户名
   * @param pass 数据库密码
   * @param schema 数据库 schema（数据库名）
   * @param pool_size 连接池大小（最大连接数）
   *
   * @note 构造函数会预先创建 pool_size 个连接并放入池中。
   */
  MysqlConnPool(const std::string &url, const std::string &user,
                const std::string &pass, const std::string &schema,
                int pool_size);

  /**
   * @brief 析构函数
   *
   * 释放所有数据库连接，清空连接池。
   */
  ~MysqlConnPool();

  /**
   * @brief 获取一个数据库连接
   *
   * 调用该方法会从池中取出一个可用连接。
   * 如果没有可用连接，会阻塞等待，直到有连接归还。
   *
   * @return
   *        sql_connection: 正常情况下
   *        nullptr: 如果连接池已经关闭
   */
  sql_conn_uptr_t get_connection();

  /**
   * @brief 归还数据库连接
   *
   * 当使用完数据库连接后，需要调用该方法将连接放回池中，
   * 以便其他线程复用。
   *
   * @param con 待归还的数据库连接
   */
  void return_connection(sql_conn_uptr_t con);

  /**
   * @brief 关闭连接池
   *
   * 将池标记为关闭状态，唤醒所有等待的线程。
   * 之后调用 get_connection() 将返回 nullptr。
   */
  void close();

private:
  void check_connection();
  void check_connection_pro();
  bool reconnect(std::chrono::steady_clock::time_point now);

private:
  std::string url_;                  ///< 数据库地址
  std::string user_;                 ///< 用户名
  std::string pass_;                 ///< 密码
  std::string schema_;               ///< schema（数据库名）
  int pool_size_;                    ///< 连接池大小
  std::queue<sql_conn_uptr_t> pool_; ///< 连接池队列
  std::mutex mutex_;                 ///< 互斥锁，保证线程安全
  std::condition_variable cond_; ///< 条件变量，用于连接等待与通知
  std::atomic<bool> b_stop_;     ///< 标记连接池是否关闭
  std::thread check_thread_;
  int32_t fail_count_;
};
#endif