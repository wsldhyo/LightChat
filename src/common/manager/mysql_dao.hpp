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
  /**
   * @brief 检查给定用户名对应的邮箱是否与传入 email 相同
   *
   * 从连接池获取连接，执行：
   *   SELECT email FROM user WHERE name = ?
   * 遍历结果集，只要有一条记录的 email 与传入的 email 相等就返回 true，
   * 否则返回 false。
   *
   * @param name  要查询的用户名
   * @param email 要验证的邮箱
   * @return bool  如果存在 name 且对应的某条记录 email == email 则返回
   * true；否则返回 false。
   *
   * @note
   * - 若无法获取连接或发生 SQL 异常，函数返回 false 并打印错误。
   * - 如果用户名在表中不存在，则返回 false。
   */
  bool check_email(const std::string &name, const std::string &email);

  /**
   * @brief 更新指定用户名的密码
   *
   * 执行 SQL:
   *   UPDATE user SET pwd = ? WHERE name = ?
   *
   * @param name   用户名
   * @param newpwd 新密码（应为哈希后字符串）
   * @return bool  当前实现返回 true（无论 updateCount），失败时返回 false
   *
   * @note
   * - 建议将返回值用于表示实际更新到的行数或更新是否真正发生。
   * - 当前实现没有根据 updateCount 决定返回值，建议改进（见建议）。
   */
  bool update_pwd(const std::string &name, const std::string &newpwd);

  /**
   * @brief 校验邮箱对应用户的密码并填充用户信息
   *
   * 根据 email 查询用户记录（SELECT * FROM user WHERE email = ?），
   * 读取数据库中的密码字段并与传入的 pwd 比较；如果匹配则填充 userInfo 并返回
   * true， 否则返回 false。
   *
   * @param email     用于查找用户的邮箱
   * @param pwd 待校验的密码（应与数据库中存储的密码哈希比较，或调用方负责哈希）
   * @param userInfo  输出参数：成功匹配时填充用户信息
   * @return bool     密码匹配且 userInfo 填充成功返回 true，否则返回 false
   *
   * @note
   * - 函数使用 Defer 确保连接被归还
   * - 建议在查询结果循环内直接填充 userInfo
   * 并返回，以避免使用已经“走过”结果集的 res 后再访问列。
   */
  bool check_pwd(const std::string &email, const std::string &pwd,
                 UserInfo &userInfo);
  /**
   * @brief 根据 uid 查询用户信息
   *
   * 从 user 表读取 uid 行并填充 UserInfo，返回 std::optional<UserInfo>：
   * - 有记录时返回填充后的 UserInfo
   * - 无记录或异常时返回 std::nullopt
   *
   * @param uid 用户 ID
   * @return std::optional<UserInfo> 查询到用户则返回包含 UserInfo 的
   * optional，否则返回 std::nullopt
   *
   * @note
   * - 已使用 Defer 保证连接归还，这是推荐的模式。
   * - 只取第一条记录并 break。
   */
  std::optional<UserInfo> get_user(int32_t uid);
  /**
   * @brief 根据用户名查询用户信息
   *
   * 根据 name 查询 user 表并返回 std::optional<UserInfo>：
   * - 找到用户返回填充的 UserInfo
   * - 未找到或异常返回 std::nullopt
   *
   * @param name 用户名
   * @return std::optional<UserInfo> 查询到用户则返回 UserInfo，否则返回
   * std::nullopt
   */
  std::optional<UserInfo> get_user(std::string const &name);

  /**
   * @brief 在 friend_apply 表中新增好友申请记录。
   *
   * 从连接池获取数据库连接，执行插入操作：
   * - 若 (from_uid, to_uid) 组合不存在，则插入新记录；
   * - 若已存在，则执行无副作用的更新（保持幂等）。
   *
   * @param from  发起申请的用户 ID。
   * @param to    接收申请的用户 ID。
   *
   * @return true 操作成功（至少影响一行记录）；
   *         false 获取连接失败或 SQL 执行异常。
   *
   * @note 异常时会捕获 SQLException 并输出错误日志。
   */
  bool add_friend_apply(int const from, int const to);

  /**
   * @brief 获取指定用户的好友申请列表
   *
   * 从数据库中查询接收人为指定用户 (to_uid) 的好友申请记录，并返回结果列表。
   * 查询结果会包含申请人 ID、申请状态、姓名、昵称、性别等信息。
   * 支持通过起始 ID 和返回条数限制结果范围。
   *
   * @param touid      接收好友申请的用户 ID
   * @param applyList  输出参数，用于存储查询到的好友申请信息列表
   * @param begin      起始 ID，只查询 ID 大于该值的记录（用于分页或增量查询）
   * @param limit      最大返回条数（限制查询结果数量）
   *
   * @return true  查询成功并填充了 applyList
   * @return false 查询失败（数据库连接为空或 SQL 执行异常）
   *
   * @note
   * - 使用连接池获取数据库连接，查询完成后会自动归还；
   * - SQL 内部通过 JOIN 语句关联 user 表获取申请人信息；
   * - 若查询失败，会捕获 sql::SQLException 并输出错误信息；
   * - applyList 中的元素为 std::shared_ptr<ApplyInfo>，由本函数构造并返回。
   * @todo  将返回值使用optional包装，让接口语义更明确
   */
  bool get_apply_list(int touid,
                      std::vector<std::shared_ptr<ApplyInfo>> &applyList,
                      int begin, int limit);


  
  bool auth_friend_apply(int const from, int const to);

  bool add_friend(int const from, int const to, std::string const& back_name);

  bool get_friend_list(int self_uid,
                       std::vector<std::shared_ptr<UserInfo>> friend_list);
private:
  std::unique_ptr<MysqlConnPool> pool_;
};
#endif