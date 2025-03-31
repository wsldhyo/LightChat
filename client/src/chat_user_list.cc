#include "chat_user_list.hpp"
#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QScrollBar>
#include <QWheelEvent>
ChatUserList::ChatUserList(QWidget *_parent /*nullptr */)
    : QListWidget(_parent) {
  Q_UNUSED(_parent);
  // 初始禁用滚动条
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  // 安装事件过滤器
  this->viewport()->installEventFilter(this);
}

bool ChatUserList::eventFilter(QObject *watched, QEvent *_event) {
  // 检查事件是否是鼠标悬浮进入或离开
  if (watched == this->viewport()) {
    if (_event->type() == QEvent::Enter) {
      // 鼠标悬浮，显示滚动条
      this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else if (_event->type() == QEvent::Leave) {
      // 鼠标离开，隐藏滚动条
      this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
  }

  // 检查事件是否是鼠标滚轮事件
  if (watched == this->viewport() && _event->type() == QEvent::Wheel) {
    QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(_event);
    int numDegrees = wheelEvent->angleDelta().y() / 8;
    int numSteps = numDegrees / 15; // 计算滚动步数

    // 设置滚动幅度， 即鼠标每次滚动下拉显示多少
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() -
                                        numSteps);

    // 检查是否滚动到底部
    QScrollBar *scrollBar = this->verticalScrollBar();
    int maxScrollValue = scrollBar->maximum();
    int currentValue = scrollBar->value();
    // int pageSize = 10; // 每页加载的联系人数量
    if (maxScrollValue - currentValue <= 0) {
      // 滚动到底部，加载新的联系人
      qDebug() << "load more chat user";
      // 发送信号通知聊天界面加载更多聊天内容
      emit sig_loading_chat_user();
    }
    return true; // 停止事件传递
  }

  // 其他事件由基类处理
  return QListWidget::eventFilter(watched, _event);
}