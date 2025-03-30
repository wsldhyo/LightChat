#include "user_manager.hpp"

UserManager::UserManager() {}
UserManager::~UserManager() {}
void UserManager::set_name(QString const &_name) { name_ = _name; }
void UserManager::set_uid(int _uid) { uid_ = _uid; }
void UserManager::set_token(QString const &_token) { token_ = _token; }