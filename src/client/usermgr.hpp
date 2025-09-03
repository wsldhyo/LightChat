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
  void SetName(QString name);
  void SetUid(int uid);
  void SetToken(QString token);

private:
  UserMgr();
  QString _name;
  QString _token;
  int _uid;
};

#endif // USERMGR_H
