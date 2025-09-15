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
  QString const &get_token() const;
  std::vector<std::shared_ptr<ApplyInfo>> const &get_apply_list() const;

  void set_user_info(std::shared_ptr<UserInfo> user_info);

  // 登录状态下，收到的好友申请
  void add_apply_list(std::shared_ptr<ApplyInfo> apply_info);

  // 添加登录时的服务器返回的好友申请列表
  void append_apply_list(QJsonArray array);

  bool already_apply(int uid);

private:
  UserMgr();
  QString token_;
  std::vector<std::shared_ptr<ApplyInfo>> apply_list_; // TODO 改成hash map
  std::shared_ptr<UserInfo> user_info_;
};

#endif // USERMGR_H
