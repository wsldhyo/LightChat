#include "usermgr.hpp"
#include "user_data.hpp"
UserMgr::UserMgr() {}

UserMgr::~UserMgr() {}

//void UserMgr::set_name(QString name) { name_ = name; }

QString const &UserMgr::get_name() const { return user_info_->_name; }

//void UserMgr::set_uid(int uid) { uid_ = uid; }

int UserMgr::get_uid() const { return user_info_->_uid; }

void UserMgr::set_token(QString token) { token_ = token; }

QString const &UserMgr::get_token() const { return token_; }

std::vector<std::shared_ptr<ApplyInfo>> const &UserMgr::get_apply_list() const {
  return apply_list_;
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