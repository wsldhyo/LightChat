#ifndef USER_DATA_HPP
#define USER_DATA_HPP
#include <QJsonArray>
#include <QJsonObject>
#include <QString>
#include <memory>
#include <vector>
class SearchInfo {
public:
  SearchInfo(int uid, QString name, QString nick, QString desc, int sex,
             QString icon);
  SearchInfo(SearchInfo const &) = default;
  SearchInfo(SearchInfo &&) = default;
  int uid_;
  QString name_;
  QString nick_;
  QString desc_;
  int sex_;
  QString icon_;
};

class AddFriendApply {
public:
  AddFriendApply(int from_uid, QString name, QString desc, QString icon,
                 QString nick, int sex);
  int from_uid_;
  QString name_;
  QString desc_;
  QString icon_;
  QString nick_;
  int sex_;
};

struct ApplyInfo {
  ApplyInfo(int uid, QString name, QString desc, QString icon, QString nick,
            int sex, int status);

  ApplyInfo(std::shared_ptr<AddFriendApply> add_info);

  void set_icon(QString head);

  int uid_;
  QString name_;
  QString desc_;
  QString icon_;
  QString nick_;
  int sex_;
  int status_;
};

// 好友认证信息
struct AuthInfo {
  AuthInfo(int uid, QString name, QString nick, QString icon, int sex);

  int uid_;
  QString name_;
  QString nick_;
  QString icon_;
  int sex_;
};

// 好友认证答复
struct AuthRsp {
  AuthRsp(int peer_uid, QString peer_name, QString peer_nick, QString peer_icon,
          int peer_sex);

  int uid_;
  QString name_;
  QString nick_;
  QString icon_;
  int sex_;
};

struct TextChatData;
struct FriendInfo {
  FriendInfo(int uid, QString name, QString nick, QString icon, int sex,
             QString desc, QString back, QString last_msg = "");

  FriendInfo(std::shared_ptr<AuthInfo> auth_info);

  FriendInfo(std::shared_ptr<AuthRsp> auth_rsp);

  void
  append_chat_msgs(const std::vector<std::shared_ptr<TextChatData>> text_vec);

  int uid_;
  QString name_;
  QString nick_;
  QString icon_;
  int sex_;
  QString desc_;
  QString back_;
  QString last_msg_;
  std::vector<std::shared_ptr<TextChatData>> chat_msgs_;
};

struct UserInfo {
  UserInfo(int uid, QString name, QString nick, QString icon, int sex,
           QString last_msg = "", QString desc = "");

  UserInfo(std::shared_ptr<AuthInfo> auth);

  UserInfo(int uid, QString name, QString icon);

  UserInfo(std::shared_ptr<AuthRsp> auth);

  UserInfo(std::shared_ptr<SearchInfo> search_info);

  UserInfo(std::shared_ptr<FriendInfo> friend_info);

  int uid_;
  QString name_;
  QString nick_;
  QString icon_;
  int sex_;
  QString desc_;
  QString last_msg_; // 聊天会话列表项中显示的最后一条聊天消息
  std::vector<std::shared_ptr<TextChatData>> chat_msgs_;
};

// 每条聊天信息
struct TextChatData {
  TextChatData(QString msg_id, QString msg_content, int fromuid, int touid);

  QString msg_id_;
  QString msg_content_;
  // TODO from和t是否可以优化掉，使用TextChatMsg里的信息
  int from_uid_;
  int to_uid_;
};

// 与该用户的所有聊天信息
struct TextChatMsg {
  TextChatMsg(int fromuid, int touid, QJsonArray arrays);

  int to_uid_;
  int from_uid_;
  std::vector<std::shared_ptr<TextChatData>> chat_msgs_;
};
#endif