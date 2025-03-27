#include "mysql_manager.hpp"

MySqlManager::MySqlManager() {}

MySqlManager::~MySqlManager() {}

int MySqlManager::register_user(std::string const &_name,
                                std::string const &_email,
                                std::string const &_pwd) {
  return dao_.register_user(_name, _email, _pwd);
}
bool MySqlManager::check_email(const std::string &_name,
                               const std::string &_email) {
  return dao_.check_email(_name, _email);
}
bool MySqlManager::check_pwd(const std::string &_name,
                               const std::string &_pwd, UserInfo& _user_info) {
  return dao_.check_pwd(_name, _pwd, _user_info);
}

bool MySqlManager::update_pwd(const std::string &_name,
                              const std::string &_newpwd) {
  return dao_.update_pwd(_name, _newpwd);
}