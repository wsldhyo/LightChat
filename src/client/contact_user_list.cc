#include "contact_user_list.hpp"
#include "contact_user_item.hpp"
#include "group_tip_item.hpp"
#include <QDebug>
#include <QRandomGenerator>
#include <QScrollBar>
#include <QWheelEvent>
ContactUserList::ContactUserList(QWidget *parent) {
  Q_UNUSED(parent);

  // 默认关闭滚动条（鼠标悬浮时显示）
  this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

  // 安装事件过滤器，用于控制滚动条显示与滚动逻辑
  this->viewport()->installEventFilter(this);

  // 模拟从数据库/后端获取数据并加载联系人
  addContactUserList();

  // 点击条目时触发槽函数
  connect(this, &QListWidget::itemClicked, this,
          &ContactUserList::slot_item_clicked);
}

void ContactUserList::ShowRedPoint(bool bshow /*= true*/) {
  add_friend_item_->ShowRedPoint(bshow);
}

// ------------------ 模拟数据 ------------------
static std::vector<QString> strs = {
    "hello world !", "nice to meet u", "New year，new life",
    "You have to love yourself",
    "My love is written in the wind ever since the whole world is you"};

static std::vector<QString> heads = {":/icons/head_1.jpg", ":/icons/head_2.jpg",
                                     ":/icons/head_3.jpg", ":/icons/head_4.jpg",
                                     ":/icons/head_5.jpg"};

static std::vector<QString> names = {"llfc", "zack",   "golang", "cpp",
                                     "java", "nodejs", "python", "rust"};

void ContactUserList::addContactUserList() {
  // 添加“新的朋友”分组提示
  auto *groupTip = new GroupTipItem();
  QListWidgetItem *item = new QListWidgetItem;
  item->setSizeHint(groupTip->sizeHint());
  this->addItem(item);
  this->setItemWidget(item, groupTip);
  // 不可选中
  item->setFlags(item->flags() & ~Qt::ItemIsSelectable);

  // 添加“新的朋友”条目
  add_friend_item_ = new ContactUserItem();
  add_friend_item_->setObjectName("new_friend_item");
  add_friend_item_->SetInfo(0, tr("新的朋友"), ":/icons/add_friend.png");
  add_friend_item_->SetItemType(ListItemType::APPLY_FRIEND_ITEM);

  QListWidgetItem *add_item = new QListWidgetItem;
  add_item->setSizeHint(add_friend_item_->sizeHint());
  this->addItem(add_item);
  this->setItemWidget(add_item, add_friend_item_);

  // 默认选中“新的朋友”条目
  this->setCurrentItem(add_item);

  // 添加“联系人”分组提示
  auto *groupCon = new GroupTipItem();
  groupCon->SetGroupTip(tr("联系人"));
  group_item_ = new QListWidgetItem;
  group_item_->setSizeHint(groupCon->sizeHint());
  this->addItem(group_item_);
  this->setItemWidget(group_item_, groupCon);
  group_item_->setFlags(group_item_->flags() & ~Qt::ItemIsSelectable);

  // 模拟加载 13 个联系人
  for (int i = 0; i < 13; i++) {
    int randomValue = QRandomGenerator::global()->bounded(100); // 随机数
    int str_i = randomValue % strs.size();
    int head_i = randomValue % heads.size();
    int name_i = randomValue % names.size();

    auto *con_user_wid = new ContactUserItem();
    con_user_wid->SetInfo(0, names[name_i], heads[head_i]);

    QListWidgetItem *item = new QListWidgetItem;
    item->setSizeHint(con_user_wid->sizeHint());
    this->addItem(item);
    this->setItemWidget(item, con_user_wid);
  }
}

bool ContactUserList::eventFilter(QObject *watched, QEvent *event) {
  if (watched == this->viewport()) {
    if (event->type() == QEvent::Enter) {
      // 鼠标悬浮显示滚动条
      this->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    } else if (event->type() == QEvent::Leave) {
      // 鼠标离开隐藏滚动条
      this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    }
  }

  // 鼠标滚轮事件
  if (watched == this->viewport() && event->type() == QEvent::Wheel) {
    QWheelEvent *wheelEvent = static_cast<QWheelEvent *>(event);
    int numDegrees = wheelEvent->angleDelta().y() / 8;
    int numSteps = numDegrees / 15; // 计算滚动步数

    // 滚动列表
    this->verticalScrollBar()->setValue(this->verticalScrollBar()->value() -
                                        numSteps);

    // 判断是否滚动到底部
    QScrollBar *scrollBar = this->verticalScrollBar();
    if (scrollBar->maximum() - scrollBar->value() <= 0) {
      qDebug() << "load more contact user";
      emit sig_loading_contact_user(); // 触发加载更多
    }

    return true; // 截断事件传递
  }

  return QListWidget::eventFilter(watched, event);
}

void ContactUserList::slot_item_clicked(QListWidgetItem *item) {
  QWidget *widget = this->itemWidget(item);
  if (!widget) {
    qDebug() << "slot item clicked widget is nullptr";
    return;
  }

  // 转换为自定义的 ListItemBase
  ListItemBase *customItem = qobject_cast<ListItemBase *>(widget);
  if (!customItem) {
    qDebug() << "slot item clicked widget is nullptr";
    return;
  }

  auto itemType = customItem->GetItemType();
  if (itemType == ListItemType::INVALID_ITEM ||
      itemType == ListItemType::GROUP_TIP_ITEM) {
    qDebug() << "slot invalid item clicked ";
    return;
  }

  if (itemType == ListItemType::APPLY_FRIEND_ITEM) {
    qDebug() << "apply friend item clicked ";
    emit sig_switch_apply_friend_page();
    return;
  }

  if (itemType == ListItemType::CONTACT_USER_ITEM) {
    qDebug() << "contact user item clicked ";
    emit sig_switch_friend_info_page();
    return;
  }
}
