#ifndef MYSQL_MANAGER_HPP
#define MYSQL_MANAGER_HPP
#include "../common/singleton.hpp"
#include "mysql_dao.hpp"
class MySqlManager : public Singleton<MySqlManager> {
public:
  ~MySqlManager();
  int register_user(std::string const& _name, std::string const& _email,
              std::string const& _pwd);

private:
  friend class Singleton<MySqlManager>;
  MySqlManager();
  MysqlDao dao_;
};

#endif