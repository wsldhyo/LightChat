#include "mysql_conn_pool.hpp"
#include "utility/defer.hpp"
MysqlConnPool::MysqlConnPool(const std::string &url, const std::string &user,
                             const std::string &pass, const std::string &schema,
                             int poolSize)
    : url_(url), user_(user), pass_(pass), schema_(schema),
      pool_size_(poolSize), b_stop_(false), fail_count_(0) {
  try {
    for (int i = 0; i < pool_size_; ++i) {

      // 获取数据库驱动实例，并通过该实例连接数据库
      sql::mysql::MySQL_Driver *driver =
          sql::mysql::get_mysql_driver_instance();
      auto *con = driver->connect(url_, user_, pass_);
      con->setSchema(schema_); // 设置所使用的数据库
      pool_.push(std::make_unique<SqlConnection>(
          con, std::chrono::steady_clock::now()));
      std::cout << "mysql connection init success" << std::endl;
    }

    check_thread_ = std::thread([this]() {
      int count = 0;
      while (!b_stop_) {
        if (count >= 60) {
          count = 0;
          check_connection_pro();
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
        count++;
      }
    });
  } catch (sql::SQLException &e) {
    // 处理异常
    std::cout << "mysql pool init failed" << std::endl;
  }
}

MysqlConnPool::~MysqlConnPool() {

  // 先停止后台线程，并阻止从池中取出连接等操作
  {
    std::lock_guard<std::mutex> lk(mutex_);
    b_stop_.store(true);
    cond_.notify_all();
  }
  if (check_thread_.joinable()) {
    check_thread_.join();
  }

  // 安全地移除私有连接
  {
    std::lock_guard<std::mutex> lk(mutex_);
    while (!pool_.empty()) {
      auto conn = std::move(pool_.front());
      pool_.pop();
      if (conn && conn->con_) {
        try {
          conn->con_->close();
        } catch (...) { /* 忽略析构期间异常 */
        }
      }
    }
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

void MysqlConnPool::check_connection() {
  std::lock_guard<std::mutex> guard(mutex_);
  int poolsize = pool_.size();
  auto now = std::chrono::steady_clock::now();
  for (int i = 0; i < poolsize; i++) {
    auto con = std::move(pool_.front());
    pool_.pop();
    Defer defer([this, &con]() { pool_.push(std::move(con)); });

    if (now - con->last_operate_time_ < std::chrono::seconds(5)) {
      continue;
    }

    try {
      std::unique_ptr<sql::Statement> stmt(con->con_->createStatement());
      stmt->executeQuery("SELECT 1");
      con->last_operate_time_ = now;
      // std::cout << "execute timer alive query , cur is " << timestamp <<
      // std::endl;
    } catch (sql::SQLException &e) {
      std::cout << "Error keeping connection alive: " << e.what() << std::endl;
      sql::mysql::MySQL_Driver *driver =
          sql::mysql::get_mysql_driver_instance();
      auto *newcon = driver->connect(url_, user_, pass_);
      newcon->setSchema(schema_);
      con->con_.reset(newcon);
      con->last_operate_time_ = now;
    }
  }
}

void MysqlConnPool::check_connection_pro() {
  std::cout << "check mydql connection\n";
  // 先读取目标连接数
  size_t target_count;
  {
    std::lock_guard<std::mutex> guard(mutex_);
    target_count = pool_.size();
  }

  size_t processed = 0; // 当前已经处理的数量

  // 超时检查
  auto now = std::chrono::steady_clock::now();

  bool healthy{true};
  while (processed < target_count) {
    sql_conn_uptr_t con;
    // 取出连接
    {
      std::lock_guard<std::mutex> guard(mutex_);
      if (pool_.empty()) {
        break;
      }
      con = std::move(pool_.front());
      pool_.pop();
    }

    healthy = true;
    // 检查连接, 距离上次操作间隔5s就操作一次数据库，防止数据库断开连接
    if (now - con->last_operate_time_ >= std::chrono::seconds(5)) {
      try {
        std::unique_ptr<sql::Statement> stmt(con->con_->createStatement());
        stmt->executeQuery("SELECT 1");
        con->last_operate_time_ = now;
        std::cout << "check connection to mysql success!\n";
      } catch (sql::SQLException &e) {
        std::cout << "Error keeping connection alive: " << e.what()
                  << std::endl;
        healthy = false;
        fail_count_++;
      }
    }

    // 如果连接有效，就将连接放回池中
    if (healthy) {
      std::cout << "mysql connection healthy\n";
      std::lock_guard<std::mutex> guard(mutex_);
      pool_.push(std::move(con));
      cond_.notify_one();
    }

    ++processed;
  }

  // 如果有异常连接，导致池中连接数量减少，就尝试重连，恢复池中连接数量
  while (fail_count_ > 0) {
    auto b_res = reconnect(now);
    if (b_res) {
      if (fail_count_ > 0) {
        fail_count_--;
      }
    } else {
      break;
    }
  }
}

bool MysqlConnPool::reconnect(std::chrono::steady_clock::time_point now) {
  std::cout << "reconnect to mysql\n";
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (pool_.size() >= 5) {
      fail_count_ = 0;
      std::cout << "mysql connection resume!\n";
      return true;
    }
  }

  try {

    sql::mysql::MySQL_Driver *driver = sql::mysql::get_mysql_driver_instance();
    auto *con = driver->connect(url_, user_, pass_);
    con->setSchema(schema_);

    auto newCon = std::make_unique<SqlConnection>(con, now);
    {
      std::lock_guard<std::mutex> guard(mutex_);
      pool_.push(std::move(newCon));
    }
    std::cout << "mysql connection reconnect success" << std::endl;
    return true;

  } catch (sql::SQLException &e) {
    std::cout << "Reconnect failed, error is " << e.what() << std::endl;
    return false;
  }
}
