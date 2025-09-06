#include "usermgr.hpp"

UserMgr::~UserMgr() {}

void UserMgr::set_name(QString name) { name_ = name; }

void UserMgr::set_uid(int uid) { uid_ = uid; }

void UserMgr::set_token(QString token) { token_ = token; }

QString const &UserMgr::get_name() const { return name_; }

UserMgr::UserMgr() {}