#include "mysql_manager.hpp"

MySqlManager::MySqlManager() {}

MySqlManager::~MySqlManager() {}

int MySqlManager::register_user(std::string const &_name,
                                std::string const &_email,
                                std::string const &_pwd
                                ) {
  return dao_.register_user(_name, _email,
                            _pwd);
}