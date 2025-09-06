#ifndef USERMGR_HPP
#define USERMGR_HPP
#include "utility/singleton.hpp"
#include <QObject>
#include <QString>
#include <memory>

class UserMgr : public QObject,
                public Singleton<UserMgr>,
                public std::enable_shared_from_this<UserMgr> {
  Q_OBJECT
public:
  friend class Singleton<UserMgr>;
  ~UserMgr();
  void set_name(QString name);
  void set_uid(int uid);
  void set_token(QString token);
  QString const& get_name()const;

private:
  UserMgr();
  QString name_;
  QString token_;
  int uid_;
};

#endif // USERMGR_H
