#include "new_friend_apply_list.hpp"
#include <QScrollBar>
#include <QWheelEvent>
NewFriendApplyList::NewFriendApplyList(QWidget *parent) {
  Q_UNUSED(parent);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  // 安装事件过滤器
  this->viewport()->installEventFilter(this);
}

bool NewFriendApplyList::eventFilter(QObject *watched, QEvent *event) {
  if (watched == this->viewport()) {
    if (event->type() == QEvent::Enter) {
      this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else if (event->type() == QEvent::Leave) {
      this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    } else if (event->type() == QEvent::MouseButtonPress) {
      emit sig_show_search(false);
    } else if (event->type() == QEvent::Wheel) {
      QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
      int numDegrees = wheelEvent->angleDelta().y() / 8;
      int numSteps = numDegrees / 15; // 每步 15 度
      this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() -
                                          numSteps);
      return true; // 阻止事件继续传播
    }
  }

  return QListWidget::eventFilter(watched, event);
}