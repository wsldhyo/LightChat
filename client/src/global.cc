#include "global.hpp"
#include <QStyle>

QString gate_url_prefix {""};

std::function<void(QWidget *)> repolish = [](QWidget* _widget) {
    _widget->style()->unpolish(_widget);   // 卸载之前的样式
    _widget->style()->polish(_widget);     // 刷新新的  样式
};


void xor_string(const QString &_src, QString &_des){
    _des = _src;
    int length = _src.length();
    length %= 255;
    for (int i = 0; i < length; ++i) {
        // QChar内部使用绝大部分采用2字节编码的UTF-16，因此unicode()返回的是ushort
        _des[i] = QChar(static_cast<ushort>(_src[i].unicode() ^ static_cast<short>(length)));
    }
}