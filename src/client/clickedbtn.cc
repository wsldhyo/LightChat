#include "clickedbtn.hpp"
#include "client_globalvar.hpp"
#include <QVariant>
ClickedBtn::ClickedBtn(QWidget *parent) : QPushButton(parent) {
  setCursor(Qt::PointingHandCursor); // 设置光标为小手

}

ClickedBtn::~ClickedBtn() {}

void ClickedBtn::SetState(QString normal, QString hover, QString press) {
  hover_ = hover;
  normal_ = normal;
  press_ = press;
  setProperty("state", normal);
  g_repolish(this);
  update();
}

void ClickedBtn::enterEvent(QEvent *event) {
  setProperty("state", hover_);
  g_repolish(this);
  update();
  QPushButton::enterEvent(event);
}

void ClickedBtn::leaveEvent(QEvent *event) {
  setProperty("state", normal_);
  g_repolish(this);
  update();
  QPushButton::leaveEvent(event);
}

void ClickedBtn::mousePressEvent(QMouseEvent *event) {
  setProperty("state", press_);
  g_repolish(this);
  update();
  QPushButton::mousePressEvent(event);
}

void ClickedBtn::mouseReleaseEvent(QMouseEvent *event) {
  setProperty("state", hover_);
  g_repolish(this);
  update();
  QPushButton::mouseReleaseEvent(event);
}