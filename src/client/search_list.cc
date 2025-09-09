#include "search_list.hpp"
#include "add_user_item.hpp"
#include "find_success_dlg.hpp"
#include "tcp_manager.hpp"
#include <QScrollBar>
#include <QWheelEvent>
SearchList::SearchList(QWidget *parent)
    : QListWidget(parent), find_dlg_(nullptr), search_edit_(nullptr),
      send_pending_(false) {
  Q_UNUSED(parent);
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  // 安装事件过滤器
  this->viewport()->installEventFilter(this);
  //添加条目
  add_tip_item();
  create_connection();
}

void SearchList::close_find_dlg() {
  if(find_dlg_){
    find_dlg_->hide();
    find_dlg_ = nullptr;
  }
}

void SearchList::set_search_edit(QWidget *edit) {}

bool SearchList::eventFilter(QObject *watched, QEvent *event) {
  // 检查事件是否是鼠标悬浮进入或离开
  if (watched == this->viewport()) {
    if (event->type() == QEvent::Enter) {
      // 鼠标悬浮，显示滚动条
      this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else if (event->type() == QEvent::Leave) {
      // 鼠标离开，隐藏滚动条
      this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
  }

  // 检查事件是否是鼠标滚轮事件
  if (watched == this->viewport() && event->type() == QEvent::Wheel) {
    QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
    int numDegrees = wheelEvent->angleDelta().y() / 8;
    int numSteps = numDegrees / 15; // 计算滚动步数

    // 设置滚动幅度
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() -
                                        numSteps);

    return true; // 停止事件传递
  }

  return QListWidget::eventFilter(watched, event);
}

void SearchList::wait_pending(bool pending) {}
void SearchList::add_tip_item() {
  auto *invalid_item = new QWidget();
  QListWidgetItem *item_tmp = new QListWidgetItem;
  // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
  item_tmp->setSizeHint(QSize(250, 10));
  this->addItem(item_tmp);
  invalid_item->setObjectName("invalid_item");
  this->setItemWidget(item_tmp, invalid_item);
  item_tmp->setFlags(item_tmp->flags() & ~Qt::ItemIsSelectable);
  qDebug() << "add user item";
  auto *add_user_item = new AddUserItem();
  QListWidgetItem *item = new QListWidgetItem;
  // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
  item->setSizeHint(add_user_item->sizeHint());
  this->addItem(item);
  this->setItemWidget(item, add_user_item);
}

void SearchList::create_connection() {

  //连接点击的信号和槽
  connect(this, &QListWidget::itemClicked, this,
          &SearchList::slot_item_clicked);

  //连接搜索条目
  connect(TcpMgr::getinstance().get(), &TcpMgr::sig_user_search, this,
          &SearchList::slot_user_search);
}

void SearchList::slot_item_clicked(QListWidgetItem *item) {
  QWidget *widget = this->itemWidget(item); //获取自定义widget对象
  if (!widget) {
    qDebug() << "slot item clicked widget is nullptr";
    return;
  }

  // 对自定义widget进行操作， 将item 转化为基类ListItemBase
  ListItemBase *customItem = qobject_cast<ListItemBase *>(widget);
  if (!customItem) {
    qDebug() << "slot item clicked widget is nullptr";
    return;
  }

  auto itemType = customItem->GetItemType();
  if (itemType == ListItemType::INVALID_ITEM) {
    qDebug() << "slot invalid item clicked ";
    return;
  }

  if (itemType == ListItemType::ADD_USER_TIP_ITEM) {

    // todo ...
    find_dlg_ = std::make_shared<FindSuccessDlg>(this);
    auto si = std::make_shared<SearchInfo>(0, "llfc", "llfc",
                                           "hello , my friend!", 0, "head_1.jpg");
    (std::dynamic_pointer_cast<FindSuccessDlg>(find_dlg_))->set_search_info(si);
    find_dlg_->show();
    return;
  }

  //上面的类型都不满足，清楚弹出框
  close_find_dlg();
}

void SearchList::slot_user_search(std::shared_ptr<SearchInfo> si) {}