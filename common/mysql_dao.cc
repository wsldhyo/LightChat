#include "mysql_dao.hpp"
#include "../common/defer.hpp"
#include "config_manager.hpp"
#include <iostream>
MySqlPool::MySqlPool(const std::string &url, const std::string &user,
                     const std::string &pass, const std::string &schema,
                     int poolSize)
    : url_(url), user_(user), pass_(pass), schema_(schema),
      pool_size_(poolSize), b_stop_(false) {
  std::cout << "mysql url" << url << std::endl;
  try {
    for (int i = 0; i < pool_size_; ++i) {
      // 获取driver驱动
      sql::mysql::MySQL_Driver *driver =
          sql::mysql::get_mysql_driver_instance();
      auto *con = driver->connect(url_, user_, pass_);
      // 设置数据库名
      con->setSchema(schema_);
      // 获取当前时间戳
      auto currentTime = std::chrono::system_clock::now().time_since_epoch();
      // 将时间戳转换为秒
      long long timestamp =
          std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
      pool_.push(std::make_unique<SqlConnection>(con, timestamp));
    }

    check_thread_ = std::thread([this]() {
      while (!b_stop_) {
        check_connection();
        std::this_thread::sleep_for(std::chrono::seconds(60));
      }
    });

    check_thread_.detach();
  } catch (sql::SQLException &e) {
    // 处理异常
    std::cout << "mysql pool init failed, error is " << e.what() << std::endl;
  }
}

MySqlPool::~MySqlPool() {
  std::unique_lock<std::mutex> lock(mutex_);
  while (!pool_.empty()) {
    pool_.pop();
  }
}

void MySqlPool::check_connection() {
  std::lock_guard<std::mutex> guard(mutex_);
  int poolsize = pool_.size();
  // 获取当前时间戳
  auto currentTime = std::chrono::system_clock::now().time_since_epoch();
  // 将时间戳转换为秒
  long long timestamp =
      std::chrono::duration_cast<std::chrono::seconds>(currentTime).count();
  for (int i = 0; i < poolsize; i++) {
    auto con = std::move(pool_.front());
    pool_.pop();
    Defer defer([this, &con]() { pool_.push(std::move(con)); });

    if (timestamp - con->last_operate_time < 60) {
      continue;
    }

    // 如果距离上次操作时间大于阈值时间，为避免连接断开，去执行简单的sql语句
    try {
      std::unique_ptr<sql::Statement> stmt(con->connection->createStatement());
      stmt->executeQuery("SELECT 1");
      con->last_operate_time = timestamp;
      // std::cout << "execute timer alive query , cur is " << timestamp <<
      // std::endl;
    } catch (sql::SQLException &e) {
      std::cout << "Error keeping connection alive: " << e.what() << std::endl;
      // 重新创建连接并替换旧的连接
      sql::mysql::MySQL_Driver *driver =
          sql::mysql::get_mysql_driver_instance();
      auto *newcon = driver->connect(url_, user_, pass_);
      newcon->setSchema(schema_);
      con->connection.reset(newcon);
      con->last_operate_time = timestamp;
    }
  }
}

std::unique_ptr<SqlConnection> MySqlPool::get_connection() {
  std::unique_lock<std::mutex> lock(mutex_);
  cond_.wait(lock, [this] {
    if (b_stop_) {
      return true;
    }
    return !pool_.empty();
  });
  if (b_stop_) {
    return nullptr;
  }
  std::unique_ptr<SqlConnection> con(std::move(pool_.front()));
  pool_.pop();
  return con;
}

void MySqlPool::return_connection(std::unique_ptr<SqlConnection> _con) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (b_stop_) {
    return;
  }
  pool_.push(std::move(_con));
  cond_.notify_one();
}

void MySqlPool::close() {
  b_stop_ = true;
  cond_.notify_all();
}

MysqlDao::MysqlDao() {
  auto sp_cfg = ConfigManager::get_instance();
  auto &cfg = *sp_cfg;
  const auto &host = cfg["Mysql"]["host"];
  const auto &port = cfg["Mysql"]["port"];
  const auto &pwd = cfg["Mysql"]["pwd"];
  const auto &schema = cfg["Mysql"]["schema"];
  const auto &user = cfg["Mysql"]["user"];
  // 根据配置信息，初始化连接池
  pool_.reset(new MySqlPool(host + ":" + port, user, pwd, schema, 5));
}

MysqlDao::~MysqlDao() { pool_->close(); }

int MysqlDao::register_user(const std::string &name, const std::string &email,
                            const std::string &pwd) {
  auto con = pool_->get_connection();
  try {
    if (con == nullptr) {
      return false;
    }
    // 准备调用存储过程
    std::unique_ptr<sql::PreparedStatement> stmt(
        con->connection->prepareStatement("CALL reg_user(?,?,?,@result)"));
    // 设置输入参数
    stmt->setString(1, name);
    stmt->setString(2, email);
    stmt->setString(3, pwd);

    // 由于PreparedStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值

    // 执行存储过程
    stmt->execute();
    // 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
    // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
    std::unique_ptr<sql::Statement> stmtResult(
        con->connection->createStatement());

    // 执行查询
    std::unique_ptr<sql::ResultSet> res(
        stmtResult->executeQuery("SELECT @result AS result"));
    // 获取查询结果
    if (res->next()) {
      int result = res->getInt("result");
      std::cout << "Result: " << result << std::endl;
      pool_->return_connection(std::move(con));
      return result;
    }
    std::cout << "no result" << std::endl;
    pool_->return_connection(std::move(con));
    return -1;
  } catch (sql::SQLException &e) {
    pool_->return_connection(std::move(con));
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return -1;
  }
}

int MysqlDao::register_user(std::string const &_name, std::string const &_email,
                            std::string const &_pwd, std::string const &_icon) {

  auto con = pool_->get_connection();
  if (con == nullptr) {
    return false;
  }

  Defer defer([this, &con] { pool_->return_connection(std::move(con)); });

  try {
    // 开始事务
    con->connection->setAutoCommit(false);
    // 执行第一个数据库操作，根据email查找用户
    //  准备查询语句

    std::unique_ptr<sql::PreparedStatement> pstmt_email(
        con->connection->prepareStatement(
            "SELECT 1 FROM user WHERE email = ?"));

    // 绑定参数
    pstmt_email->setString(1, _email);

    // 执行查询
    std::unique_ptr<sql::ResultSet> res_email(pstmt_email->executeQuery());

    auto email_exist = res_email->next();
    if (email_exist) {
      con->connection->rollback();
      std::cout << "email " << _email << " exist";
      return 0;
    }

    // 准备查询用户名是否重复
    std::unique_ptr<sql::PreparedStatement> pstmt_name(
        con->connection->prepareStatement("SELECT 1 FROM user WHERE name = ?"));

    // 绑定参数
    pstmt_name->setString(1, _name);

    // 执行查询
    std::unique_ptr<sql::ResultSet> res_name(pstmt_name->executeQuery());

    auto name_exist = res_name->next();
    if (name_exist) {
      con->connection->rollback();
      std::cout << "name " << _name << " exist";
      return 0;
    }

    // 准备更新用户id
    std::unique_ptr<sql::PreparedStatement> pstmt_upid(
        con->connection->prepareStatement("UPDATE user_id SET id = id + 1"));

    // 执行更新
    pstmt_upid->executeUpdate();

    // 获取更新后的 id 值
    std::unique_ptr<sql::PreparedStatement> pstmt_uid(
        con->connection->prepareStatement("SELECT id FROM user_id"));
    std::unique_ptr<sql::ResultSet> res_uid(pstmt_uid->executeQuery());
    int newId = 0;
    // 处理结果集
    if (res_uid->next()) {
      newId = res_uid->getInt("id");
    } else {
      std::cout << "select id from user_id failed" << std::endl;
      con->connection->rollback();
      return -1;
    }

    // 插入user信息
    std::unique_ptr<sql::PreparedStatement> pstmt_insert(
        con->connection->prepareStatement(
            "INSERT INTO user (uid, name, _email, pwd, nick, icon) "
            "VALUES (?, ?, ?, ?,?,?)"));
    pstmt_insert->setInt(1, newId);
    pstmt_insert->setString(2, _name);
    pstmt_insert->setString(3, _email);
    pstmt_insert->setString(4, _pwd);
    pstmt_insert->setString(5, _name);
    pstmt_insert->setString(6, _icon);
    // 执行插入
    pstmt_insert->executeUpdate();
    // 提交事务
    con->connection->commit();
    std::cout << "newuser insert into user success" << std::endl;
    return newId;
  } catch (sql::SQLException &e) {
    // 如果发生错误，回滚事务
    if (con) {
      con->connection->rollback();
    }
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return -1;
  }
}

bool MysqlDao::check_email(const std::string &name, const std::string &_email) {
  // 从池中取出一个连接
  auto con = pool_->get_connection();
  try {
    if (con == nullptr) {
      return false;
    }

    // 准备查询用户对应的邮箱的SQL语句
    std::unique_ptr<sql::PreparedStatement> pstmt(
        con->connection->prepareStatement(
            "SELECT email FROM user WHERE name = ?"));

    // 绑定上面SQL中?占位的参数
    pstmt->setString(1, name);

    // 执行查询
    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

    // 遍历结果集
    while (res->next()) {
      std::cout << "Check email: " << res->getString("email") << std::endl;
      if (_email != res->getString("email")) {
        pool_->return_connection(std::move(con));
        return false;
      }
      pool_->return_connection(std::move(con));
      return true;
    }
    return false;
  } catch (sql::SQLException &e) {
    pool_->return_connection(std::move(con));
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}

bool MysqlDao::update_pwd(const std::string &name, const std::string &newpwd) {
  auto con = pool_->get_connection();
  try {
    if (con == nullptr) {
      return false;
    }

    // 准备查询用户对应的邮箱的SQL语句
    std::unique_ptr<sql::PreparedStatement> pstmt(
        con->connection->prepareStatement(
            "UPDATE user SET pwd = ? WHERE name = ?"));

    // 绑定上面SQL中?占位的参数
    pstmt->setString(2, name);
    pstmt->setString(1, newpwd);

    // 执行更新
    int updateCount = pstmt->executeUpdate();

    std::cout << "Updated rows: " << updateCount << std::endl;
    pool_->return_connection(std::move(con));
    return true;
  } catch (sql::SQLException &e) {
    pool_->return_connection(std::move(con));
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}

bool MysqlDao::check_pwd(const std::string &_email, const std::string &pwd,
                         UserInfo &userInfo) {
  // 取出一个Mysql连接
  auto con = pool_->get_connection();
  if (con == nullptr) {
    return false;
  }

  Defer defer([this, &con]() { pool_->return_connection(std::move(con)); });

  try {

    // 准备SQL语句
    std::unique_ptr<sql::PreparedStatement> pstmt(
        con->connection->prepareStatement(
            "SELECT * FROM user WHERE email = ?"));
    // 将username替换为要查询的用户名
    pstmt->setString(1, _email);

    // 执行查询
    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    std::string origin_pwd = "";
    // 遍历结果集
    while (res->next()) {
      origin_pwd = res->getString("pwd");
      // 输出查询到的密码
      std::cout << "Password: " << origin_pwd << std::endl;
      break;
    }

    if (pwd != origin_pwd) {
      return false;
    }
    userInfo.name = res->getString("name");
    userInfo.email = res->getString("email");
    userInfo.uid = res->getInt("uid");
    userInfo.pwd = origin_pwd;
    return true;
  } catch (sql::SQLException &e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}
