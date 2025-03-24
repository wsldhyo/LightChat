#ifndef GLOBAL_HPP
#define GLOBAL_HPP
#include <QWidget>
#include <functional>
#include <QString>
extern QString gate_url_prefix;

// 刷新QSS的全局函数对象
extern std::function<void(QWidget*)> repolish;

/**
 * @brief 将字符串的每一个字符与字符串长度做或操作，实现简单的加密效果
 * 
 * @param _src 
 * @param _des 
 */
extern void xor_string(QString const& _src, QString & _des);

#endif