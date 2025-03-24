#include "pwd_visible_lbl.hpp"
#include "global.hpp"
#include <QDebug>
#include <QMouseEvent>
PwdVisibleLbl::PwdVisibleLbl(QWidget *_parent /* nullptr*/)
    : QLabel(_parent), curstate_(PwdVisibleState::NORMAL) {}

void PwdVisibleLbl::mousePressEvent(QMouseEvent *_event) {
  if (_event->button() == Qt::LeftButton) {
    if (curstate_ == PwdVisibleState::NORMAL) {
      qDebug() << "clicked , change to selected hover: " << selected_hover_;
      curstate_ = PwdVisibleState::SELECTED;
      setProperty("state", selected_hover_);
      repolish(this);
      update();
    } else {
      qDebug() << "clicked , change to normal hover: " << normal_hover_;
      curstate_ = PwdVisibleState::NORMAL;
      setProperty("state", normal_hover_);
      repolish(this);
      update();
    }
    emit clicked();
  }
  // 调用基类的mousePressEvent以保证正常的事件处理
  QLabel::mousePressEvent(_event);
}

void PwdVisibleLbl::enterEvent(QEvent *_event) {
  if (curstate_ == PwdVisibleState::NORMAL) {
    qDebug() << "enter , change to normal hover: " << normal_hover_;
    setProperty("state", normal_hover_);
    repolish(this);
    update();
  } else {
    qDebug() << "enter , change to selected hover: " << selected_hover_;
    setProperty("state", selected_hover_);
    repolish(this);
    update();
  }
  QLabel::enterEvent(_event);
}

void PwdVisibleLbl::leaveEvent(QEvent *_event) {
  if (curstate_ == PwdVisibleState::NORMAL) {
    qDebug() << "leave , change to normal : " << normal_;
    setProperty("state", normal_);
    repolish(this);
    update();
  } else {
    qDebug() << "leave , change to normal hover: " << selected_;
    setProperty("state", selected_);
    repolish(this);
    update();
  }
  QLabel::leaveEvent(_event);
}

void PwdVisibleLbl::set_state(QString _normal /*""*/, QString _hover /*""*/,
                              QString _press /*""*/, QString _select /*""*/,
                              QString _select_hover /*""*/,
                              QString _select_press /*""*/) {
  normal_ = _normal;
  normal_hover_ = _hover;
  normal_press_ = _press;
  selected_ = _select;
  selected_hover_ = _select_hover;
  selected_press_ = _select_press;
  setProperty("state", _normal);
  repolish(this);
}
PwdVisibleState PwdVisibleLbl::get_state() { return curstate_; }