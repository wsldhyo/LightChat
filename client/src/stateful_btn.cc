#include "stateful_btn.hpp"
#include "global.hpp"
#include <QVariant>
StatefulBtn::StatefulBtn(QWidget *_parent /*nullptr*/) : QPushButton(_parent) {
  setCursor(Qt::PointingHandCursor); // 光标样式为小手
}

StatefulBtn::~StatefulBtn() {}

void StatefulBtn::set_state(QString const _normal, QString const _hover,
                            QString const _press) {
  normal_ = _normal;
  hover_ = _hover;
  press_ = _press;
  setProperty("state", normal_);
  repolish(this);
  update();
}

void StatefulBtn::enterEvent(QEvent *_e) {

}

void StatefulBtn::leaveEvent(QEvent *_e) {

}

void StatefulBtn::mousePressEvent(QMouseEvent *_e) {

}

void StatefulBtn::mouseReleaseEvent(QMouseEvent *_e) {

}
