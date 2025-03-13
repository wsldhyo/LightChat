#include "global.hpp"
#include <QStyle>
std::function<void(QWidget *)> repolish = [](QWidget* _widget) {
    _widget->style()->unpolish(_widget);   // 卸载之前的样式
    _widget->style()->polish(_widget);     // 刷新新的  样式
};