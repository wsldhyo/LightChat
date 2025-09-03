#ifndef GLOBAL_VAR_HPP
#define GLOBAL_VAR_HPP
#include <QString>
#include <functional>
class QWidget;
// 刷新样式表的lambda表达式
extern std::function<void(QWidget *)> g_repolish;

/**
 * @brief 将字符串的每一个字符与字符串长度做或操作，实现简单的加密效果
 *
 * @param _src 待加密的字符串
 * @return 加密后的字符串
 */
extern QString xor_string(const QString &_src);

extern QString g_gate_url_prefix;

#endif