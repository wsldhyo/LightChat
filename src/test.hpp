#ifndef TESTW_HPP 
#define TESTW_HPP
#include <qt5/QtWidgets/QWidget>

class TestW : public QWidget
{
    QOBJECT_H
    Q_CLASSINFO("Author", "wsldhyo")
public:
    TestW(QWidget* _parent = nullptr);
};
#endif