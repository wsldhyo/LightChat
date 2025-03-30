#ifndef USER_MANAGER_HPP
#define USER_MANAGER_HPP
#include <QObject>
#include "../common/singleton.hpp"

class UserManager: public QObject ,public Singleton<UserManager>{
    Q_OBJECT
public:
    ~UserManager();
    void set_name(QString const& _name);
    void set_uid(int _uid);
    void set_token(QString const& _token);
private:
    friend class Singleton<UserManager>;
    UserManager();

    QString name_;
    int uid_;
    QString token_;

};
#endif