#include "mysql_dao.hpp"
#include "utility/defer.hpp"
#include "config_manager.hpp"

MysqlDao::MysqlDao() {
  // 读取配置文件，获取数据库相关信息
  auto cfg = ConfigManager::getinstance();
  const auto &host = (*cfg)["Mysql"]["host"];
  const auto &port = (*cfg)["Mysql"]["port"];
  const auto &pwd = (*cfg)["Mysql"]["pwd"];
  const auto &schema = (*cfg)["Mysql"]["schema"];
  const auto &user = (*cfg)["Mysql"]["user"];
  // 创建数据库连接
  pool_.reset(new MysqlConnPool(host + ":" + port, user, pwd, schema, 5));
}

MysqlDao::~MysqlDao() { pool_->close(); }

int MysqlDao::reg_user(const std::string &name, const std::string &email,
                       const std::string &pwd) {
  auto con = pool_->get_connection();
  try {
    if (con == nullptr) {
      pool_->return_connection(std::move(con));
      return false;
    }
    // 准备调用存储过程
    std::unique_ptr<sql::PreparedStatement> stmt(
        con->prepareStatement("CALL reg_user(?,?,?,@result)"));
    // 设置输入参数
    stmt->setString(1, name);
    stmt->setString(2, email);
    stmt->setString(3, pwd);

    // 由于PreparedStatement不直接支持注册输出参数，我们需要使用会话变量或其他方法来获取输出参数的值

    // 执行存储过程
    stmt->execute();
    // 如果存储过程设置了会话变量或有其他方式获取输出参数的值，你可以在这里执行SELECT查询来获取它们
    // 例如，如果存储过程设置了一个会话变量@result来存储输出结果，可以这样获取：
    std::unique_ptr<sql::Statement> stmtResult(con->createStatement());
    std::unique_ptr<sql::ResultSet> res(
        stmtResult->executeQuery("SELECT @result AS result"));
    if (res->next()) {
      int result = res->getInt("result");
      std::cout << "Result: " << result << '\n';
      pool_->return_connection(std::move(con));
      return result;
    }
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

bool MysqlDao::check_email(const std::string &name, const std::string &email) {
  auto con = pool_->get_connection();
  try {
    if (con == nullptr) {
      pool_->return_connection(std::move(con));
      return false;
    }

    // 准备查询语句
    std::unique_ptr<sql::PreparedStatement> pstmt(
        con->prepareStatement("SELECT email FROM user WHERE name = ?"));

    // 绑定参数
    pstmt->setString(1, name);

    // 执行查询
    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());

    // 遍历结果集
    bool match = false;

    while (res->next()) {
      std::string dbEmail = res->getString("email");
      std::cout << "Check Email: " << dbEmail << std::endl;

      if (email == dbEmail) {
        match = true; // 只要有一个匹配就设为 true
        break;        // 可以直接结束循环
      }
    }

    pool_->return_connection(std::move(con));
    return match;
  } catch (sql::SQLException &e) {
    pool_->return_connection(std::move(con));
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
  return true;
}

bool MysqlDao::update_pwd(const std::string &name, const std::string &newpwd) {
  auto con = pool_->get_connection();
  try {
    if (con == nullptr) {
      pool_->return_connection(std::move(con));
      return false;
    }

    // 准备查询语句
    std::unique_ptr<sql::PreparedStatement> pstmt(
        con->prepareStatement("UPDATE user SET pwd = ? WHERE name = ?"));

    // 绑定参数
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

bool MysqlDao::check_pwd(const std::string &email, const std::string &pwd,
                         UserInfo &userInfo) {
  auto con = pool_->get_connection();
  if (con == nullptr) {
    return false;
  }

  Defer defer([this, &con]() { pool_->return_connection(std::move(con)); });

  try {

    // 准备SQL语句
    std::unique_ptr<sql::PreparedStatement> pstmt(
        con->prepareStatement("SELECT * FROM user WHERE email = ?"));
    pstmt->setString(1, email); // 将username替换为你要查询的用户名

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

std::optional<UserInfo> MysqlDao::get_user(int32_t uid) {
  auto con = pool_->get_connection();
  if (con == nullptr) {
    return std::optional<UserInfo>{};
  }

  Defer defer([this, &con]() { pool_->return_connection(std::move(con)); });

  try {

    // 准备SQL语句
    std::unique_ptr<sql::PreparedStatement> pstmt(
        con->prepareStatement("SELECT * FROM user WHERE uid = ?"));
    pstmt->setString(1,
                     std::to_string(uid)); // 将username替换为你要查询的用户名

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
    return std::make_optional<UserInfo>(
        UserInfo{res->getString("name"), res->getString("email"),
                 res->getInt("uid"), origin_pwd});
  } catch (sql::SQLException &e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return std::optional<UserInfo>{};
  }
}