
#include "user_manager.hpp"
#include "session.hpp"

UserManager::UserManager() {}
UserManager::~UserManager() {}

std::shared_ptr<Session> UserManager::get_session(int uid) {
  std::lock_guard<std::mutex> lock(session_mtx_);
  auto iter = uid_to_session_.find(uid);
  if (iter == uid_to_session_.end()) {
    return nullptr;
  }

  return iter->second;
}

void UserManager::set_user_session(int uid, std::shared_ptr<Session> session) {
  std::lock_guard<std::mutex> lock(session_mtx_);
  uid_to_session_[uid] = session;
}

void UserManager::remove_user_session(int uid) {
  auto uid_str = std::to_string(uid);
  //因为再次登录可能是其他服务器，所以会造成本服务器删除key，其他服务器注册key的情况
  // 有可能其他服务登录，本服删除key造成找不到key的情况
  // RedisMgr::GetInstance()->Del(USERIPPREFIX + uid_str);

  {
    std::lock_guard<std::mutex> lock(session_mtx_);
    uid_to_session_.erase(uid);
  }
}
