#include "mysql_manager.hpp"

MysqlMgr::MysqlMgr() {}

MysqlMgr::~MysqlMgr() {}

int MysqlMgr::reg_user(const std::string &name, const std::string &email,
                       const std::string &pwd) {
  return dao_.reg_user(name, email, pwd);
}

bool MysqlMgr::check_email(const std::string &name, const std::string &email) {
  return dao_.check_email(name, email);
}

bool MysqlMgr::update_pwd(const std::string &name, const std::string &newpwd) {
  return dao_.update_pwd(name, newpwd);
}

bool MysqlMgr::check_pwd(const std::string &email, const std::string &pwd,
                         UserInfo &userInfo) {
  return dao_.check_pwd(email, pwd, userInfo);
}

std::optional<UserInfo> MysqlMgr::get_user(int32_t uid) { return dao_.get_user(uid); }

std::optional<UserInfo> MysqlMgr::get_user(std::string const& name) { return dao_.get_user(name); }