#include "timer_btn.hpp"
#include <QDebug>
#include <QMouseEvent>
TimerBtn::TimerBtn(QWidget *parent) : QPushButton(parent), _counter(10) {
  _timer = new QTimer(this);
  
  connect(_timer, &QTimer::timeout, [this]() {
    // 定时调用回调，每次回调counter减一 
    _counter--;
    if (_counter <= 0) {
       // counter减为0，计时结束，按钮使能
      _timer->stop();
      _counter = 10;
      this->setText("获取");
      this->setEnabled(true);
      return;
    }
    // 计时为未结束，显示当前倒计时
    this->setText(QString::number(_counter));
  });
}

TimerBtn::~TimerBtn() { _timer->stop(); }

void TimerBtn::mouseReleaseEvent(QMouseEvent *e) {
  if (e->button() == Qt::LeftButton) {
    // 在这里处理鼠标左键释放事件
    qDebug() << "MyButton was released!";
    this->setEnabled(false);   // 倒计时结束前禁用按钮
    this->setText(QString::number(_counter));
    _timer->start(1000);   // 启动定时器，开始倒计时，间隔时间为1000ms
    emit clicked();
  }
  // 调用基类的mouseReleaseEvent以确保正常的事件处理（如点击效果）
  QPushButton::mouseReleaseEvent(e);
}