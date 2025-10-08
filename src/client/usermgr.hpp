#ifndef USERMGR_HPP
#define USERMGR_HPP
#include "utility/singleton.hpp"
#include <QJsonArray>
#include <QObject>
#include <QString>
#include <memory>
#include <unordered_map>
class ApplyInfo;
class UserInfo;
class AuthRsp;
class AuthInfo;
class FriendInfo;

class UserMgr : public QObject,
                public Singleton<UserMgr>,
                public std::enable_shared_from_this<UserMgr> {
  Q_OBJECT
public:
  friend class Singleton<UserMgr>;
  ~UserMgr();
  // void set_name(QString name);
  QString const &get_name() const;
  // void set_uid(int uid);
  int get_uid() const;
  void set_token(QString token);
  QString const &get_icon() const;
  QString const &get_token() const;
  std::vector<std::shared_ptr<ApplyInfo>> const &get_apply_list() const;

  void set_user_info(std::shared_ptr<UserInfo> user_info);

  // 登录状态下，收到的好友申请
  void add_apply_list(std::shared_ptr<ApplyInfo> apply_info);

  // 添加登录时的服务器返回的好友申请列表
  void append_apply_list(QJsonArray array);

  bool already_apply(int uid);
  // 添加服务器返回的好友列表
  void append_friend_list(QJsonArray array);
  // 同意添加对方为好友后，根据同意请求的回包，将对方添加为好友
  void add_friend(std::shared_ptr<AuthRsp> auth_rsp);
  // 收到对方同意成为好友的答复后，将对方添加为好友
  void add_friend(std::shared_ptr<AuthInfo> auth_info);
  // 根据uid检查对方是否是自己的好友
  bool check_friend_by_id(int uid);
  // 根据uid获取好友信息
  std::shared_ptr<FriendInfo> get_friend_infO_by_id(int uid);

  std::vector<std::shared_ptr<FriendInfo>> get_chat_list_per_page();
  bool is_load_chat_finished();
  void update_chat_loaded_count();
  std::vector<std::shared_ptr<FriendInfo>> get_conlist_per_page();
  void update_contact_loaded_count();
  bool is_load_contact_finished();

private:
  UserMgr();
  QString token_;
  std::vector<std::shared_ptr<ApplyInfo>> apply_list_; // TODO 改成hash map
  std::shared_ptr<UserInfo> user_info_;
  QMap<int, std::shared_ptr<FriendInfo>> friend_map_;

  std::vector<std::shared_ptr<FriendInfo>> friend_list_; // TODO 无必要

  int chat_loaded_; // 聊天会话项目起始索引
  int contact_loaded_; // 联系人项起始索引
};

#endif // USERMGR_H
