#ifndef GLOBAL_HPP
#define GLOBAL_HPP
#include <QWidget>
#include <functional>
#include <QString>
extern QString gate_url_prefix;

// 刷新QSS的全局函数对象
extern std::function<void(QWidget*)> repolish;


#endif