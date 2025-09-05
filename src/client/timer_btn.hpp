#ifndef TIMER_BTN_HPP
#define TIMER_BTN_HPP
#include <QPushButton>
#include <QTimer>

class TimerBtn : public QPushButton {
public:
  TimerBtn(QWidget *parent = nullptr);
  ~TimerBtn();

  // 重写mouseReleaseEvent
  virtual void mouseReleaseEvent(QMouseEvent *e) override;

private:
  QTimer *_timer;
  int _counter;
};

#endif // TIMERBTN_H