#ifndef TIME_BTN_HPP
#define TIME_BTN_HPP
#include <QPushButton>
#include <QTimer>

class TimerBtn : public QPushButton {
    Q_OBJECT
public:
    TimerBtn(QWidget *parent = nullptr);
    ~ TimerBtn();
    // 鼠标释放后开始计时
    void mouseReleaseEvent(QMouseEvent *e) override;
private:
    QTimer  *_timer;
    int _counter;
};
#endif