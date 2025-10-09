#include "usermgr.hpp"
#include "client_constant.hpp"
#include "user_data.hpp"
UserMgr::UserMgr() : user_info_(nullptr), chat_loaded_(0), contact_loaded_(0) {}

UserMgr::~UserMgr() {}

// void UserMgr::set_name(QString name) { name_ = name; }

QString const &UserMgr::get_name() const { return user_info_->_name; }

// void UserMgr::set_uid(int uid) { uid_ = uid; }

int UserMgr::get_uid() const { return user_info_->_uid; }

void UserMgr::set_token(QString token) { token_ = token; }

QString const &UserMgr::get_token() const { return token_; }

QString const &UserMgr::get_icon() const { return user_info_->_icon; }

std::vector<std::shared_ptr<ApplyInfo>> const &UserMgr::get_apply_list() const {
  return apply_list_;
}

std::shared_ptr<UserInfo> const UserMgr::get_user_info() const {
  return user_info_;
}

void UserMgr::set_user_info(std::shared_ptr<UserInfo> user_info) {
  user_info_ = user_info;
}

void UserMgr::append_apply_list(QJsonArray array) {
  qDebug() << "append apply list.";
  //将QJsonArray的每个元素反序列化为ApplyInfo并添加到apply_lsit_中
  for (const QJsonValue &value : array) {
    auto name = value["name"].toString();
    auto desc = value["desc"].toString();
    auto icon = value["icon"].toString();
    auto nick = value["nick"].toString();
    auto sex = value["sex"].toInt();
    auto uid = value["uid"].toInt();
    auto status = value["status"].toInt();
    auto info =
        std::make_shared<ApplyInfo>(uid, name, desc, icon, nick, sex, status);
    qDebug() << value;
    apply_list_.push_back(info);
  }
  qDebug() << "append friend list finished.";
}

void UserMgr::add_apply_list(std::shared_ptr<ApplyInfo> apply_info) {
  apply_list_.push_back(apply_info);
}

bool UserMgr::already_apply(int uid) {
  auto find_res = std::find_if(apply_list_.begin(), apply_list_.end(),
                               [uid](std::shared_ptr<ApplyInfo> apply_info) {
                                 return apply_info->_uid == uid;
                               });
  return find_res != apply_list_.end();
}

void UserMgr::append_friend_list(QJsonArray array) {
  // 遍历 QJsonArray 并输出每个元素
  for (const QJsonValue &value : array) {
    auto name = value["name"].toString();
    auto desc = value["desc"].toString();
    auto icon = value["icon"].toString();
    auto nick = value["nick"].toString();
    auto sex = value["sex"].toInt();
    auto uid = value["uid"].toInt();
    auto back = value["back"].toString();

    auto info =
        std::make_shared<FriendInfo>(uid, name, nick, icon, sex, desc, back);
    friend_list_.push_back(info);
    friend_map_.insert(uid, info);
  }
}

void UserMgr::add_friend(std::shared_ptr<AuthRsp> auth_rsp) {
  friend_map_[auth_rsp->_uid] = std::make_shared<FriendInfo>(auth_rsp);
}

void UserMgr::add_friend(std::shared_ptr<AuthInfo> auth_info) {
  friend_map_[auth_info->_uid] = std::make_shared<FriendInfo>(auth_info);
}

bool UserMgr::check_friend_by_id(int uid) {
  return friend_map_.find(uid) != friend_map_.end();
}

std::shared_ptr<FriendInfo> UserMgr::get_friend_infO_by_id(int uid) {
  auto find_res = friend_map_.find(uid);
  if (find_res == friend_map_.end()) {
    return nullptr;
  }
  return find_res.value();
}

std::vector<std::shared_ptr<FriendInfo>> UserMgr::get_chat_list_per_page() {

  std::vector<std::shared_ptr<FriendInfo>> friend_list;
  int begin = chat_loaded_;
  int end = begin + CHAT_COUNT_PER_PAGE;

  if (begin >= friend_list_.size()) {
    return friend_list;
  }

  if (end > friend_list_.size()) {
    friend_list = std::vector<std::shared_ptr<FriendInfo>>(
        friend_list_.begin() + begin, friend_list_.end());
    return friend_list;
  }

  friend_list = std::vector<std::shared_ptr<FriendInfo>>(
      friend_list_.begin() + begin, friend_list_.begin() + end);
  return friend_list;
}

bool UserMgr::is_load_chat_finished() {
  if (chat_loaded_ >= friend_list_.size()) {
    return true;
  }

  return false;
}

void UserMgr::update_chat_loaded_count() {
  int begin = chat_loaded_;
  int end = begin + CHAT_COUNT_PER_PAGE;

  if (begin >= friend_list_.size()) {
    return;
  }

  if (end > friend_list_.size()) {
    chat_loaded_ = friend_list_.size();
    return;
  }

  chat_loaded_ = end;
}

std::vector<std::shared_ptr<FriendInfo>> UserMgr::get_conlist_per_page() {
  std::vector<std::shared_ptr<FriendInfo>> friend_list;
  int begin = contact_loaded_;
  int end = begin + CHAT_COUNT_PER_PAGE;

  if (begin >= friend_list_.size()) {
    return friend_list;
  }

  if (end > friend_list_.size()) {
    friend_list = std::vector<std::shared_ptr<FriendInfo>>(
        friend_list_.begin() + begin, friend_list_.end());
    return friend_list;
  }

  friend_list = std::vector<std::shared_ptr<FriendInfo>>(
      friend_list_.begin() + begin, friend_list_.begin() + end);
  return friend_list;
}

void UserMgr::update_contact_loaded_count() {
  int begin = contact_loaded_;
  int end = begin + CHAT_COUNT_PER_PAGE;

  if (begin >= friend_list_.size()) {
    return;
  }

  if (end > friend_list_.size()) {
    contact_loaded_ = friend_list_.size();
    return;
  }

  contact_loaded_ = end;
}

bool UserMgr::is_load_contact_finished() {
  if (contact_loaded_ >= friend_list_.size()) {
    return true;
  }

  return false;
}

void UserMgr::append_friend_chat_msg(
    int friend_id, std::vector<std::shared_ptr<TextChatData>> msgs) {
  auto find_iter = friend_map_.find(friend_id);
  if (find_iter == friend_map_.end()) {
    qDebug() << "append friend uid  " << friend_id << " not found";
    return;
  }

  find_iter.value()->AppendChatMsgs(msgs);
}