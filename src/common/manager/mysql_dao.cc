#include "mysql_dao.hpp"
#include "config_manager.hpp"
#include "utility/defer.hpp"

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

  if (con == nullptr) {
    return -1;
  }
  // 使用 Defer 自动归还数据库连接
  Defer defer([this, &con]() { pool_->return_connection(std::move(con)); });
  try {
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
      return result;
    }
    return -1;
  } catch (sql::SQLException &e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return -1;
  }
}

bool MysqlDao::check_email(const std::string &name, const std::string &email) {
  auto con = pool_->get_connection();
  if (con == nullptr) {
    return false;
  }
  // 使用 Defer 自动归还数据库连接
  Defer defer([this, &con]() { pool_->return_connection(std::move(con)); });
  try {

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

    return match;
  } catch (sql::SQLException &e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
  return true;
}

bool MysqlDao::update_pwd(const std::string &name, const std::string &newpwd) {
  auto con = pool_->get_connection();
  if (con == nullptr) {
    return false;
  }
  Defer defer([this, &con]() { pool_->return_connection(std::move(con)); });
  try {
    if (con == nullptr) {
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
    return true;
  } catch (sql::SQLException &e) {
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

// 根据 uid 查询用户信息
std::optional<UserInfo> MysqlDao::get_user(int32_t uid) {
  // 从连接池获取数据库连接
  auto con = pool_->get_connection();
  if (con == nullptr) {
    return std::nullopt; // 获取连接失败，返回空
  }

  // 使用 Defer 自动归还数据库连接
  Defer defer([this, &con]() { pool_->return_connection(std::move(con)); });

  try {
    // 创建预编译 SQL 语句
    std::unique_ptr<sql::PreparedStatement> pstmt(
        con->prepareStatement("SELECT * FROM user WHERE uid = ?"));
    pstmt->setInt(1, uid); // 设置查询参数

    // 执行查询
    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    std::optional<UserInfo> user_info;

    // 遍历结果集
    while (res->next()) {
      user_info.emplace(); // 在 optional 中创建 UserInfo 对象
      user_info->pwd = res->getString("pwd");
      user_info->email = res->getString("email");
      user_info->name = res->getString("name");
      user_info->nick = res->getString("nick");
      user_info->desc = res->getString("desc");
      user_info->sex = res->getInt("sex");
      user_info->icon = res->getString("icon");
      user_info->uid = uid;
      break; // 只取第一条记录
    }
    return user_info; // 返回查询结果
  } catch (sql::SQLException &e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return std::nullopt;
  }
}

// 根据用户名查询用户信息
std::optional<UserInfo> MysqlDao::get_user(const std::string &name) {
  // 从连接池获取数据库连接
  auto con = pool_->get_connection();
  if (con == nullptr) {
    return std::nullopt; // 获取连接失败，返回空
  }

  // 使用 Defer 自动归还数据库连接
  Defer defer([this, &con]() { pool_->return_connection(std::move(con)); });

  try {
    // 创建预编译 SQL 语句，根据用户名查询
    std::cout << "query user name: " << name << '\n';
    std::unique_ptr<sql::PreparedStatement> pstmt(
        con->prepareStatement("SELECT * FROM user WHERE name = ?"));
    pstmt->setString(1, name); // 设置查询参数

    // 执行查询
    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    std::optional<UserInfo> user_info;

    // 遍历结果集
    while (res->next()) {
      std::cout << "found user: " << name << '\n';
      user_info.emplace(); // 在 optional 中创建 UserInfo 对象
      user_info->pwd = res->getString("pwd");
      user_info->email = res->getString("email");
      user_info->name = res->getString("name");
      user_info->nick = res->getString("nick");
      user_info->desc = res->getString("desc");
      user_info->sex = res->getInt("sex");
      user_info->icon = res->getString("icon");
      user_info->uid = res->getInt("uid");
      break; // 只取第一条记录
    }
    return user_info;
  } catch (sql::SQLException &e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return std::nullopt;
  }
}

bool MysqlDao::add_friend_apply(int const from, int const to) {
  // 从连接池获取数据库连接
  auto con = pool_->get_connection();
  if (con == nullptr) {
    return false;
  }

  // 使用 Defer 自动归还数据库连接
  Defer defer([this, &con]() { pool_->return_connection(std::move(con)); });

  try {
    std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
        "INSERT INTO friend_apply (from_uid, to_uid) values "
        "(?,?) " // 将from和to插入表
        "ON DUPLICATE KEY UPDATE from_uid = from_uid, to_uid = "
        "to_uid ")); // (from,to)的联合主键已存在时，采用更新而非插入（可不写该条件）
    pstmt->setInt(1, from);
    pstmt->setInt(2, to);
    //执行更新
    int rowAffected = pstmt->executeUpdate();
    if (rowAffected < 0) {
      // 上面的操作理论上必定会影响表中的一行数据
      return false;
    }
    return true;
  } catch (sql::SQLException &e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}

bool MysqlDao::get_apply_list(
    int touid, std::vector<std::shared_ptr<ApplyInfo>> &applyList, int begin,
    int limit) {
  auto con = pool_->get_connection();
  if (con == nullptr) {
    return false;
  }

  Defer defer([this, &con]() { pool_->return_connection(std::move(con)); });

  try {
    // 准备SQL语句, 根据起始id和限制条数返回列表
    // 联表查询
    std::unique_ptr<sql::PreparedStatement> pstmt(con->prepareStatement(
        // 查询申请人ID、申请状态、名字  申请人昵称、性别
        "select apply.from_uid, apply.status, user.name, user.nick, user.sex "
        "from friend_apply as apply "             // 从好友申请表
        "join user on apply.from_uid = user.uid " // 关联用户表，获取申请人信息
        "where apply.to_uid = ? " // 接收人是指定用户 (参数1)
        "and apply.id > ? "       // 只取大于某个ID的记录 (参数2)
        "order by apply.id ASC "  // 按申请ID升序
        "LIMIT ? "                // 限制返回条数 (参数3)
        ));
    pstmt->setInt(1, touid); // 将uid替换为你要查询的uid
    pstmt->setInt(2, begin); // 起始id
    pstmt->setInt(3, limit); //偏移量
    // 执行查询
    std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
    // 遍历结果集
    while (res->next()) {
      auto name = res->getString("name");
      auto uid = res->getInt("from_uid");
      auto status = res->getInt("status");
      auto nick = res->getString("nick");
      auto sex = res->getInt("sex");
      auto apply_ptr =
          std::make_shared<ApplyInfo>(uid, name, "", "", nick, sex, status);
      applyList.push_back(apply_ptr);
    }
    return true;
  } catch (sql::SQLException &e) {
    std::cerr << "SQLException: " << e.what();
    std::cerr << " (MySQL error code: " << e.getErrorCode();
    std::cerr << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    return false;
  }
}