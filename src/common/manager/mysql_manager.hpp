#ifndef MYSQL_MANAGER_HPP
#define MYSQL_MANAGER_HPP
#include "mysql_dao.hpp"
#include "utility/singleton.hpp"
#include <optional>

/**
 * @brief MySQL 管理类（单例模式）
 *
 * 该类作为 MySQL 数据访问的统一入口，基于 Singleton 模式实现全局唯一实例。
 * 内部封装了 MysqlDao，用于对外提供数据库操作接口。
 *
 * 设计目的：
 * - 统一管理 MySQL 相关操作
 * - 对外隐藏 DAO 层的细节
 * - 保证全局只有一个 MySQL 管理对象，避免资源重复创建
 */
class MysqlMgr : public Singleton<MysqlMgr> {
  friend class Singleton<MysqlMgr>;

public:
  /**
   * @brief 析构函数
   *
   * 用于清理资源（当前实现无额外资源释放逻辑）。
   */
  ~MysqlMgr();

  /**
   * @brief 析构函数
   *
   * 用于清理资源（当前实现无额外资源释放逻辑）。
   */
  int reg_user(const std::string &name, const std::string &email,
              const std::string &pwd);

  bool check_email(const std::string &name, const std::string &email);

  bool update_pwd(const std::string &name, const std::string &pwd);

  bool check_pwd(const std::string &email, const std::string &pwd,
                 UserInfo &userInfo);

  std::optional<UserInfo> get_user(int32_t uid);

  std::optional<UserInfo> get_user(std::string const& name);
private:
  /**
   * @brief 构造函数（私有）
   *
   * 由于继承自 Singleton，该类禁止外部直接构造，
   * 只能通过 Singleton<MysqlMgr>::getInstance() 获取唯一实例。
   */
  MysqlMgr();

private:
  MysqlDao dao_; ///< DAO 层对象，用于实际的数据库访问
};
#endif