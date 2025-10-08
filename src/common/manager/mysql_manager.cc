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

std::optional<UserInfo> MysqlMgr::get_user(int32_t uid) {
  return dao_.get_user(uid);
}

std::optional<UserInfo> MysqlMgr::get_user(std::string const &name) {
  return dao_.get_user(name);
}

bool MysqlMgr::add_friend_apply(int const from, int const to) {
  return dao_.add_friend_apply(from, to);
}

bool MysqlMgr::get_apply_list(
    int touid, std::vector<std::shared_ptr<ApplyInfo>> &applyList, int begin,
    int limit) {

  return dao_.get_apply_list(touid, applyList, begin, limit);
}

bool MysqlMgr::auth_friend_apply(int const from, int const to) {
  return dao_.auth_friend_apply(from, to);
}

bool MysqlMgr::add_friend(int const from, int const to,
                          std::string const &back_name) {
  return dao_.add_friend(from, to, back_name);
}

bool MysqlMgr::get_friend_list(
    int self_uid, std::vector<std::shared_ptr<UserInfo>> friend_list) {
  return dao_.get_friend_list(self_uid, friend_list);
}