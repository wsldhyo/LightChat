#ifndef STATEFUL_BTN
#define STATEFUL_BTN

#include <QPushButton>

class StatefulBtn : public QPushButton {
  Q_OBJECT
public:
  StatefulBtn(QWidget *_parent = nullptr);
  ~StatefulBtn();
  void set_state(QString const _normal, QString const _hover,
                 QString const _press);
protected:
    void enterEvent(QEvent* _e)override;
    void leaveEvent(QEvent* _e)override;
    void mousePressEvent(QMouseEvent* _e)override;
    void mouseReleaseEvent(QMouseEvent* _e)override;

private:
    QString normal_;
    QString hover_;
    QString press_;
};
#endif