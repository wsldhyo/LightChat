#include "clicked_label.hpp"
#include "client_globalvar.hpp"
#include <QDebug>
#include <QMouseEvent>
ClickedLabel::ClickedLabel(QWidget *parent)
    : QLabel(parent), curstate_(ClickLbState::Normal) {
  setCursor(Qt::PointingHandCursor);
}

// 处理鼠标点击事件
void ClickedLabel::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    if (curstate_ == ClickLbState::Normal) {
      // qDebug()<<"PressEvent , change to selected press: "<< _selected_press;
      curstate_ = ClickLbState::Selected;
      setProperty("state", selected_press_);
      g_repolish(this);
      update();

    } else {
      // qDebug()<<"PressEvent , change to normal press: "<< _normal_press;
      curstate_ = ClickLbState::Normal;
      setProperty("state", normal_press_);
      g_repolish(this);
      update();
    }
    return;
  }
  // 调用基类的mousePressEvent以保证正常的事件处理
  QLabel::mousePressEvent(event);
}

void ClickedLabel::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    if (curstate_ == ClickLbState::Normal) {
      // qDebug()<<"ReleaseEvent , change to normal hover: "<< _normal_hover;
      setProperty("state", normal_hover_);
      g_repolish(this);
      update();

    } else {
      //  qDebug()<<"ReleaseEvent , change to select hover: "<< _selected_hover;
      setProperty("state", selected_hover_);
      g_repolish(this);
      update();
    }
    emit clicked();
    return;
  }
  // 调用基类的mousePressEvent以保证正常的事件处理
  QLabel::mousePressEvent(event);
}

// 处理鼠标悬停进入事件
void ClickedLabel::enterEvent(QEvent *event) {
  // 在这里处理鼠标悬停进入的逻辑
  if (curstate_ == ClickLbState::Normal) {
    // qDebug()<<"enter , change to normal hover: "<< normal_hover_;
    setProperty("state", normal_hover_);
    g_repolish(this);
    update();

  } else {
    // qDebug()<<"enter , change to selected hover: "<< selected_hover_;
    setProperty("state", selected_hover_);
    g_repolish(this);
    update();
  }

  QLabel::enterEvent(event);
}

// 处理鼠标悬停离开事件
void ClickedLabel::leaveEvent(QEvent *event) {
  // 在这里处理鼠标悬停离开的逻辑
  if (curstate_ == ClickLbState::Normal) {
    // qDebug()<<"leave , change to normal : "<< _normal;
    setProperty("state", normal_);
    g_repolish(this);
    update();

  } else {
    // qDebug()<<"leave , change to normal hover: "<< _selected;
    setProperty("state", selected_);
    g_repolish(this);
    update();
  }
  QLabel::leaveEvent(event);
}

void ClickedLabel::SetState(QString normal, QString hover, QString press,
                            QString select, QString select_hover,
                            QString select_press) {
  normal_ = normal;
  normal_hover_ = hover;
  normal_press_ = press;

  selected_ = select;
  selected_hover_ = select_hover;
  selected_press_ = select_press;

  setProperty("state", normal);
  g_repolish(this);
}

ClickLbState ClickedLabel::GetCurState() { return curstate_; }



