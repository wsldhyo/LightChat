#include "state_widget.hpp"
#include "client_globalvar.hpp"
#include <QDebug>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QStyleOption>
#include <QVBoxLayout>
#include <QVariant>
StateWidget::StateWidget(QWidget *parent)
    : QWidget(parent), curstate_(ClickLbState::Normal) {
  setCursor(Qt::PointingHandCursor);
  AddRedPoint();
}

void StateWidget::SetState(QString normal, QString hover, QString press,
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

ClickLbState StateWidget::GetCurState() { return curstate_; }

void StateWidget::ClearState() {
  curstate_ = ClickLbState::Normal;
  setProperty("state", normal_);
  g_repolish(this);
  update();
}

void StateWidget::SetSelected(bool bselected) {
  if (bselected) {
    curstate_ = ClickLbState::Selected;
    setProperty("state", selected_);
    g_repolish(this);
    update();
    return;
  }

  curstate_ = ClickLbState::Normal;
  setProperty("state", normal_);
  g_repolish(this);
  update();
  return;
}

//右上角添加红点, 表示有未读消息
void StateWidget::AddRedPoint() {
  //添加红点示意图
  red_point_ = new QLabel();
  red_point_->setObjectName("red_point");
  QVBoxLayout *layout2 = new QVBoxLayout;
  red_point_->setAlignment(Qt::AlignCenter);
  layout2->addWidget(red_point_);
  layout2->setMargin(0);
  this->setLayout(layout2);
  red_point_->setVisible(false);
}

void StateWidget::ShowRedPoint(bool show) { red_point_->setVisible(true); }

void StateWidget::paintEvent(QPaintEvent *event) {
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
  return;
}

void StateWidget::mousePressEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    if (curstate_ == ClickLbState::Selected) {
      qDebug() << "PressEvent , already to selected press: " << selected_press_;
      // emit clicked();
      //  调用基类的mousePressEvent以保证正常的事件处理
      QWidget::mousePressEvent(event);
      return;
    }

    if (curstate_ == ClickLbState::Normal) {
      qDebug() << "PressEvent , change to selected press: " << selected_press_;
      curstate_ = ClickLbState::Selected;
      setProperty("state", selected_press_);
      g_repolish(this);
      update();
    }

    return;
  }
  // 调用基类的mousePressEvent以保证正常的事件处理
  QWidget::mousePressEvent(event);
}

void StateWidget::mouseReleaseEvent(QMouseEvent *event) {
  if (event->button() == Qt::LeftButton) {
    if (curstate_ == ClickLbState::Normal) {
      // qDebug()<<"ReleaseEvent , change to normal hover: "<< normal_hover_;
      setProperty("state", normal_hover_);
      g_repolish(this);
      update();

    } else {
      // qDebug()<<"ReleaseEvent , change to select hover: "<< selected_hover_;
      setProperty("state", selected_hover_);
      g_repolish(this);
      update();
    }
    emit clicked();
    return;
  }
  // 调用基类的mousePressEvent以保证正常的事件处理
  QWidget::mousePressEvent(event);
}

void StateWidget::enterEvent(QEvent *event) {
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

  QWidget::enterEvent(event);
}

void StateWidget::leaveEvent(QEvent *event) {
  // 在这里处理鼠标悬停离开的逻辑
  if (curstate_ == ClickLbState::Normal) {
    // qDebug()<<"leave , change to normal : "<< normal_;
    setProperty("state", normal_);
    g_repolish(this);
    update();

  } else {
    // qDebug()<<"leave , change to select normal : "<< selected_;
    setProperty("state", selected_);
    g_repolish(this);
    update();
  }
  QWidget::leaveEvent(event);
}
