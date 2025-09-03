#ifndef MYSQL_DAO_HPP
#define MYSQL_DAO_HPP
#include "pool/mysql_conn_pool.hpp"
#include "utility/userinfo.hpp"
#include <optional>

/**
 * @brief MySQL 数据访问对象 (DAO)
 * 通常一张表一个DAO或一个邻域对象：
 *          UserDao → 管理 user 表 + 关联的 user_profile 表
 *          OrderDao → 管理 order 表 + order_item 表
 *
 * 该类封装了对 MySQL 数据库的访问逻辑，基于 MysqlConnPool 实现连接管理。
 * 用于隔离业务逻辑和数据库操作，提供更高层次的接口。
 *
 * 当前实现包含用户注册的接口。
 */
class MysqlDao {
public:
  /**
   * @brief 构造函数
   *
   * 从 ConfigManager 中读取 MySQL 配置，
   * 初始化连接池。
   */
  MysqlDao();

  /**
   * @brief 析构函数
   *
   * 关闭连接池，释放资源。
   */
  ~MysqlDao();

  /**
   * @brief 注册用户
   *
   * 调用数据库存储过程 `reg_user`，将用户信息插入数据库。
   * 存储过程内部通过输出参数 `@result` 返回执行结果。
   *
   * @param name 用户名
   * @param email 用户邮箱
   * @param pwd 用户密码（应保证安全存储，如哈希后传入）
   *
   * @return int
   * - 成功时返回存储过程的输出值（通常为状态码，例如 0 表示成功）
   * - 失败时返回 -1
   */
  int reg_user(const std::string &name, const std::string &email,
               const std::string &pwd);

  bool check_email(const std::string &name, const std::string &email);

  bool update_pwd(const std::string &name, const std::string &newpwd);

  bool check_pwd(const std::string &email, const std::string &pwd,
                 UserInfo &userInfo);


  std::optional<UserInfo> get_user(int32_t uid);
private:
  std::unique_ptr<MysqlConnPool> pool_;
};
#endif