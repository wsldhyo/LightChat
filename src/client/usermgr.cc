#include "usermgr.hpp"

UserMgr::~UserMgr() {}

void UserMgr::set_name(QString name) { name_ = name; }

QString const &UserMgr::get_name() const { return name_; }

void UserMgr::set_uid(int uid) { uid_ = uid; }

int UserMgr::get_uid() const { return uid_; }

void UserMgr::set_token(QString token) { token_ = token; }

QString const &UserMgr::get_token() const { return token_; }

std::vector<std::shared_ptr<ApplyInfo>> const &UserMgr::get_apply_list() const {
  return apply_list_;
}
UserMgr::UserMgr() {}