#ifndef MYSQL_MANAGER_HPP
#define MYSQL_MANAGER_HPP
#include "../common/singleton.hpp"
#include "mysql_dao.hpp"
class MySqlManager : public Singleton<MySqlManager> {
public:
  ~MySqlManager();
  int register_user(std::string const &_name, std::string const &_email,
                    std::string const &_pwd);

  bool check_email(const std::string &name, const std::string &email);

  bool check_pwd(const std::string &name, const std::string &_pwd, UserInfo & _user_info);

  bool update_pwd(const std::string &name, const std::string &pwd);

private:
  friend class Singleton<MySqlManager>;
  MySqlManager();
  MysqlDao dao_;
};

#endif