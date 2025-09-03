#include "mysql_conn_pool.hpp"
MysqlConnPool::MysqlConnPool(const std::string &url, const std::string &user,
                     const std::string &pass, const std::string &schema,
                     int poolSize)
    : url_(url), user_(user), pass_(pass), schema_(schema),
      pool_size_(poolSize), b_stop_(false) {
  try {
    for (int i = 0; i < pool_size_; ++i) {
      sql_driver_t *driver = sql::mysql::get_mysql_driver_instance();
      sql_conn_uptr_t con(driver->connect(url_, user_, pass_));
      con->setSchema(schema_);
      pool_.push(std::move(con));
    }
  } catch (sql::SQLException &e) {
    // 处理异常
    std::cout << "mysql pool init failed" << std::endl;
  }
}

MysqlConnPool::~MysqlConnPool() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (!pool_.empty()) {
    pool_.pop();
  }
}

sql_conn_uptr_t MysqlConnPool::get_connection() {
  std::unique_lock<std::mutex> lock(mutex_);
  // 等待池中有连接可用
  cond_.wait(lock, [this] {
    if (b_stop_) {
      return true;
    }
    return !pool_.empty();
  });
  // 若池被关闭，则直接返回nullptr
  if (b_stop_) {
    return nullptr;
  }
  sql_conn_uptr_t con(std::move(pool_.front()));
  pool_.pop();
  return con;
}

void MysqlConnPool::return_connection(sql_conn_uptr_t con) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (b_stop_) {
    return;
  }
  pool_.push(std::move(con));
  cond_.notify_one();
}

void MysqlConnPool::close() {
  b_stop_ = true;
  cond_.notify_all();
}
