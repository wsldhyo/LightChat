#include "user_data.hpp"
SearchInfo::SearchInfo(int uid, QString name, QString nick, QString desc,
                       int sex, QString icon)
    : uid_(uid), name_(name), nick_(nick), desc_(desc), sex_(sex), icon_(icon) {
}

AddFriendApply::AddFriendApply(int from_uid, QString name, QString desc,
                               QString icon, QString nick, int sex)
    : from_uid_(from_uid), name_(name), desc_(desc), icon_(icon), nick_(nick),
      sex_(sex) {}

ApplyInfo::ApplyInfo(int uid, QString name, QString desc, QString icon,
                     QString nick, int sex, int status)
    : uid_(uid), name_(name), desc_(desc), icon_(icon), nick_(nick), sex_(sex),
      status_(status) {}

ApplyInfo::ApplyInfo(std::shared_ptr<AddFriendApply> add_info)
    : uid_(add_info->from_uid_), name_(add_info->name_), desc_(add_info->desc_),
      icon_(add_info->icon_), nick_(add_info->nick_), sex_(add_info->sex_),
      status_(0) {}

void ApplyInfo::set_icon(QString head) { icon_ = head; }

AuthInfo::AuthInfo(int uid, QString name, QString nick, QString icon, int sex)
    : uid_(uid), name_(name), nick_(nick), icon_(icon), sex_(sex) {}

AuthRsp::AuthRsp(int peer_uid, QString peer_name, QString peer_nick,
                 QString peer_icon, int peer_sex)
    : uid_(peer_uid), name_(peer_name), nick_(peer_nick), icon_(peer_icon),
      sex_(peer_sex) {}

FriendInfo::FriendInfo(int uid, QString name, QString nick, QString icon,
                       int sex, QString desc, QString back, QString last_msg)
    : uid_(uid), name_(name), nick_(nick), icon_(icon), sex_(sex), desc_(desc),
      back_(back), last_msg_(last_msg) {}

FriendInfo::FriendInfo(std::shared_ptr<AuthInfo> auth_info)
    : uid_(auth_info->uid_), nick_(auth_info->nick_), icon_(auth_info->icon_),
      name_(auth_info->name_), sex_(auth_info->sex_) {}

FriendInfo::FriendInfo(std::shared_ptr<AuthRsp> auth_rsp)
    : uid_(auth_rsp->uid_), nick_(auth_rsp->nick_), icon_(auth_rsp->icon_),
      name_(auth_rsp->name_), sex_(auth_rsp->sex_) {}

void FriendInfo::append_chat_msgs(
    const std::vector<std::shared_ptr<TextChatData>> text_vec) {
  for (const auto &text : text_vec) {
    chat_msgs_.push_back(text);
  }
}

UserInfo::UserInfo(int uid, QString name, QString nick, QString icon, int sex,
                   QString last_msg, QString desc)
    : uid_(uid), name_(name), nick_(nick), icon_(icon), sex_(sex),
      last_msg_(last_msg), desc_(desc) {}

UserInfo::UserInfo(std::shared_ptr<AuthInfo> auth)
    : uid_(auth->uid_), name_(auth->name_), nick_(auth->nick_),
      icon_(auth->icon_), sex_(auth->sex_), last_msg_(""), desc_("") {}

UserInfo::UserInfo(int uid, QString name, QString icon)
    : uid_(uid), name_(name), icon_(icon), nick_(name_), sex_(0), last_msg_(""),
      desc_("") {}

UserInfo::UserInfo(std::shared_ptr<AuthRsp> auth)
    : uid_(auth->uid_), name_(auth->name_), nick_(auth->nick_),
      icon_(auth->icon_), sex_(auth->sex_), last_msg_("") {}

UserInfo::UserInfo(std::shared_ptr<SearchInfo> search_info)
    : uid_(search_info->uid_), name_(search_info->name_),
      nick_(search_info->nick_), icon_(search_info->icon_),
      sex_(search_info->sex_), last_msg_("") {}

UserInfo::UserInfo(std::shared_ptr<FriendInfo> friend_info)
    : uid_(friend_info->uid_), name_(friend_info->name_),
      nick_(friend_info->nick_), icon_(friend_info->icon_),
      sex_(friend_info->sex_), last_msg_("") {
  chat_msgs_ = friend_info->chat_msgs_;
}

TextChatData::TextChatData(QString msg_id, QString msg_content, int fromuid,
                           int touid)
    : msg_id_(msg_id), msg_content_(msg_content), from_uid_(fromuid),
      to_uid_(touid) {}

TextChatMsg::TextChatMsg(int fromuid, int touid, QJsonArray arrays)
    : from_uid_(fromuid), to_uid_(touid) {
  for (auto msg_data : arrays) {
    auto msg_obj = msg_data.toObject();
    auto content = msg_obj["content"].toString();
    auto msgid = msg_obj["msgid"].toString();
    auto msg_ptr =
        std::make_shared<TextChatData>(msgid, content, fromuid, touid);
    chat_msgs_.push_back(msg_ptr);
  }
}