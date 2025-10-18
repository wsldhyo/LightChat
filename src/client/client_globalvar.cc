#include "client_globalvar.hpp"
#include <QStyle>
#include <QWidget>
std::function<void(QWidget *)> g_repolish{[](QWidget *w) {
  w->style()->unpolish(w);
  w->style()->polish(w);
}};

QString g_gate_url_prefix{""};

QString xor_string(const QString &_src) {
  QString des;
  des.resize(_src.length());
  int length = _src.length();
  length %= 255;
  for (int i = 0; i < length; ++i) {
    // QChar内部使用绝大部分采用2字节编码的UTF-16，因此unicode()返回的是ushort
    des[i] = QChar(
        static_cast<ushort>(_src[i].unicode() ^ static_cast<short>(length)));
  }
  return des;
}