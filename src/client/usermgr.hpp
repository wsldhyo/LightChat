#ifndef USERMGR_HPP
#define USERMGR_HPP
#include "utility/singleton.hpp"
#include <QObject>
#include <QString>
#include <memory>
#include <vector>
class ApplyInfo;

class UserMgr : public QObject,
                public Singleton<UserMgr>,
                public std::enable_shared_from_this<UserMgr> {
  Q_OBJECT
public:
  friend class Singleton<UserMgr>;
  ~UserMgr();
  void set_name(QString name);
  QString const& get_name()const;
  void set_uid(int uid);
  int get_uid()const;
  void set_token(QString token);
  QString const& get_token()const;
  std::vector<std::shared_ptr<ApplyInfo>> const &get_apply_list() const;

private:
  UserMgr();
  QString name_;
  QString token_;
  int uid_;
  std::vector<std::shared_ptr<ApplyInfo>> apply_list_;
};

#endif // USERMGR_H
