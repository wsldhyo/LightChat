#include "chat_dialog.hpp"
#include "chat_user_wid.hpp"
#include "contact_user_item.hpp"
#include "loading_dialog.hpp"
#include "state_widget.hpp"
#include "tcp_manager.hpp"
#include "ui_chatdialog.h"
#include "user_data.hpp"
#include "usermgr.hpp"
#include <QAction>
#include <QDebug>
#include <QMouseEvent>
#include <QRandomGenerator>
ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ChatDialog), mode_(ChatUIMode::CHAT_MODE),
      state_(ChatUIMode::CHAT_MODE), b_loading_(false), cur_chat_uid_(0)

{
  ui->setupUi(this);
  ui->add_btn->SetState("normal", "hover", "press");
  set_search_edit();
  add_chat_user_list();
  ui->stackedWidget->setCurrentWidget(ui->chat_page);

  // 设置侧边栏
  QPixmap pixmap(UserMgr::getinstance()->get_icon());
  ui->side_head_lb->setPixmap(pixmap); // 将图片设置到QLabel上
  QPixmap scaledPixmap = pixmap.scaled(
      ui->side_head_lb->size(), Qt::KeepAspectRatio); // 将图片缩放到label的大小
  ui->side_head_lb->setPixmap(scaledPixmap); // 将缩放后的图片设置到QLabel上
  ui->side_head_lb->setScaledContents(
      true); // 设置QLabel自动缩放图片内容以适应大小

  ui->side_chat_lb->setProperty("state", "normal");

  ui->side_chat_lb->SetState("normal", "hover", "pressed", "selected_normal",
                             "selected_hover", "selected_pressed");

  ui->side_contact_lb->SetState("normal", "hover", "pressed", "selected_normal",
                                "selected_hover", "selected_pressed");

  add_lb_group(ui->side_chat_lb);
  add_lb_group(ui->side_contact_lb);

  // 安装事件过滤器，实现非搜索列表控件范围内的点击，将隐藏搜索列表
  this->installEventFilter(this);
  //设置聊天label选中状态
  ui->side_chat_lb->SetSelected(true);

  ui->search_list->set_search_edit(ui->search_edit);
  create_connection();
  // 加载好友申请列表
  ui->friend_apply_page->load_apply_list();
}

ChatDialog::~ChatDialog() { delete ui; }

bool ChatDialog::eventFilter(QObject *watched, QEvent *event) {
  if (event->type() == QEvent::MouseButtonPress) {
    QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
    handle_global_mouse_press(mouseEvent);
  }
  return QDialog::eventFilter(watched, event);
}

void ChatDialog::handle_global_mouse_press(QMouseEvent *event) {
  // 实现点击位置的判断和处理逻辑
  // 先判断是否处于搜索模式，如果不处于搜索模式则直接返回
  if (mode_ != ChatUIMode::SEARCH_MODE) {
    return;
  }

  // 将鼠标点击位置转换为搜索列表坐标系中的位置，并判断点击位置是否位于搜索列表控件范围内
  QPoint posInSearchList = ui->search_list->mapFromGlobal(event->globalPos());
  if (!ui->search_list->rect().contains(posInSearchList)) {
    // 不在搜索列表控件范围内的点击，将清空搜索列表的显示
    ui->search_edit->clear();
    show_search(false);
  }
}

void ChatDialog::set_search_edit() {
  // 搜索动作，放到最前面，显示为放大镜
  QAction *searchAction = new QAction(ui->search_edit);
  searchAction->setIcon(QIcon(":/icons/search.png"));
  ui->search_edit->addAction(searchAction, QLineEdit::LeadingPosition);
  ui->search_edit->setPlaceholderText(QStringLiteral("搜索"));

  // 创建一个清除动作并设置图标，显示为x删除图标
  QAction *clearAction = new QAction(ui->search_edit);
  clearAction->setIcon(QIcon(":/icons/close_transparent.png"));
  // 初始时不显示清除图标
  // 将清除动作添加到LineEdit的末尾位置
  ui->search_edit->addAction(clearAction, QLineEdit::TrailingPosition);

  // 当需要显示清除图标时，更改为实际的清除图标
  connect(
      ui->search_edit, &QLineEdit::textChanged,
      [clearAction](const QString &text) {
        if (!text.isEmpty()) {
          clearAction->setIcon(QIcon(":/icons/close_search.png"));
        } else {
          clearAction->setIcon(QIcon(
              ":/icons/close_transparent.png")); // 文本为空时，切换回透明图标
        }
      });

  // 连接清除动作的触发信号到槽函数，用于清除文本
  connect(clearAction, &QAction::triggered, [this, clearAction]() {
    ui->search_edit->clear();
    clearAction->setIcon(
        QIcon(":/icons/close_transparent.png")); // 清除文本后，切换回透明图标
    ui->search_edit->clearFocus();
    //清除按钮被按下则不显示搜索框
    show_search(false);
  });
  show_search(false);
  ui->search_edit->SetMaxLength(15);
}

static std::vector<QString> strs = {
    "hello world !", "nice to meet u", "New year，new life",
    "You have to love yourself",
    "My love is written in the wind ever since the whole world is you"};

static std::vector<QString> heads = {":/icons/head_1.jpg", ":/icons/head_2.jpg",
                                     ":/icons/head_3.jpg", ":/icons/head_4.jpg",
                                     ":/icons/head_5.jpg"};

static std::vector<QString> names = {"llfc", "zack",   "golang", "cpp",
                                     "java", "nodejs", "python", "rust"};
void ChatDialog::add_chat_user_list() {
  // 加载服务器传递过来的聊天会话项
  auto friend_list = UserMgr::getinstance()->get_chat_list_per_page();
  if (friend_list.empty() == false) {
    for (auto &friend_ele : friend_list) {
      auto find_iter = chat_items_added_.find(friend_ele->_uid);
      if (find_iter != chat_items_added_.end()) {
        continue;
      }
      auto *chat_user_wid = new ChatUserWid();
      auto user_info = std::make_shared<UserInfo>(friend_ele);
      chat_user_wid->set_user_info(user_info);
      QListWidgetItem *item = new QListWidgetItem;
      // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
      item->setSizeHint(chat_user_wid->sizeHint());
      ui->chat_user_list->addItem(item);
      ui->chat_user_list->setItemWidget(item, chat_user_wid);
      chat_items_added_.insert(friend_ele->_uid, item);
    }

    //更新已加载条目
    UserMgr::getinstance()->update_chat_loaded_count();
  }

  // 模拟测试数据
  {
    // 创建QListWidgetItem，并设置自定义的widget
    for (int i = 0; i < 13; i++) {
      int randomValue =
          QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
      int str_i = randomValue % strs.size();
      int head_i = randomValue % heads.size();
      int name_i = randomValue % names.size();

      auto *chat_user_wid = new ChatUserWid();

      chat_user_wid->set_user_info(std::make_shared<UserInfo>(
          0, names[name_i], names[name_i], heads[head_i], 0, strs[str_i]));

      QListWidgetItem *item = new QListWidgetItem;
      // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
      item->setSizeHint(chat_user_wid->sizeHint());
      // 将item加入聊天会话列表，并将item设置为自定义的widget
      ui->chat_user_list->addItem(item);
      ui->chat_user_list->setItemWidget(item, chat_user_wid);
    }
  }
}

void ChatDialog::set_select_chat_item(int uid) {
  if (ui->chat_user_list->count() <= 0) {
    return;
  }

  if (uid == 0) {
    ui->chat_user_list->setCurrentRow(0);
    QListWidgetItem *firstItem = ui->chat_user_list->item(0);
    if (!firstItem) {
      return;
    }

    //转为widget
    QWidget *widget = ui->chat_user_list->itemWidget(firstItem);
    if (!widget) {
      return;
    }

    auto con_item = qobject_cast<ChatUserWid *>(widget);
    if (!con_item) {
      return;
    }

    cur_chat_uid_ = con_item->get_user_info()->_uid;

    return;
  }

  auto find_iter = chat_items_added_.find(uid);
  if (find_iter == chat_items_added_.end()) {
    qDebug() << "uid " << uid << " not found, set curent row 0";
    ui->chat_user_list->setCurrentRow(0);
    return;
  }

  ui->chat_user_list->setCurrentItem(find_iter.value());

  cur_chat_uid_ = uid;
}

void ChatDialog::load_more_contact_user() {
  auto friend_list = UserMgr::getinstance()->get_conlist_per_page();
  if (friend_list.empty() == false) {
    for (auto &friend_ele : friend_list) {
      auto *chat_user_wid = new ContactUserItem();
      chat_user_wid->SetInfo(friend_ele->_uid, friend_ele->_name,
                             friend_ele->_icon);
      QListWidgetItem *item = new QListWidgetItem;
      // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
      item->setSizeHint(chat_user_wid->sizeHint());
      ui->con_user_list->addItem(item);
      ui->con_user_list->setItemWidget(item, chat_user_wid);
    }

    //更新已加载条目
    UserMgr::getinstance()->update_contact_loaded_count();
  }
}

void ChatDialog::load_more_chat_user() {
  auto friend_list = UserMgr::getinstance()->get_chat_list_per_page();
  if (friend_list.empty() == false) {
    for (auto &friend_ele : friend_list) {
      auto find_iter = chat_items_added_.find(friend_ele->_uid);
      if (find_iter != chat_items_added_.end()) {
        continue;
      }
      auto *chat_user_wid = new ChatUserWid();
      auto user_info = std::make_shared<UserInfo>(friend_ele);
      chat_user_wid->set_user_info(user_info);
      QListWidgetItem *item = new QListWidgetItem;
      // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
      item->setSizeHint(chat_user_wid->sizeHint());
      ui->chat_user_list->addItem(item);
      ui->chat_user_list->setItemWidget(item, chat_user_wid);
      chat_items_added_.insert(friend_ele->_uid, item);
    }

    //更新已加载条目
    UserMgr::getinstance()->update_chat_loaded_count();
  }
}

void ChatDialog::set_select_chat_page(int uid) {
  //  if (ui->chat_user_list->count() <= 0) {
  // return;
  //}

  // if (uid == 0) {
  // auto item = ui->chat_user_list->item(0);
  ////转为widget
  // QWidget *widget = ui->chat_user_list->itemWidget(item);
  // if (!widget) {
  // return;
  //}

  // auto con_item = qobject_cast<ChatUserWid *>(widget);
  // if (!con_item) {
  // return;
  //}

  ////设置信息
  // auto user_info = con_item->get_user_info();
  // ui->chat_page->set_user_info(user_info);
  // return;
  //}

  // auto find_iter = chat_items_added_.find(uid);
  // if (find_iter == chat_items_added_.end()) {
  // return;
  //}

  ////转为widget
  // QWidget *widget = ui->chat_user_list->itemWidget(find_iter.value());
  // if (!widget) {
  // return;
  //}

  ////判断转化为自定义的widget
  //// 对自定义widget进行操作， 将item 转化为基类ListItemBase
  // ListItemBase *customItem = qobject_cast<ListItemBase *>(widget);
  // if (!customItem) {
  // qDebug() << "qobject_cast<ListItemBase*>(widget) is nullptr";
  // return;
  //}

  // auto itemType = customItem->GetItemType();
  // if (itemType == ListItemType::CHAT_USER_ITEM) {
  // auto con_item = qobject_cast<ChatUserWid *>(customItem);
  // if (!con_item) {
  // return;
  //}

  ////设置信息
  // auto user_info = con_item->get_user_info();
  // ui->chat_page->set_user_info(user_info);

  // return;
  //}
}

void ChatDialog::show_search(bool bsearch) {
  if (bsearch) {
    // 搜索状态下，隐藏聊天会话列表和联系人列表，显示搜索候选列表
    ui->chat_user_list->hide();
    ui->con_user_list->hide();
    ui->search_list->show();
    mode_ = ChatUIMode::SEARCH_MODE;
  } else if (state_ == ChatUIMode::CHAT_MODE) {
    // 非搜索模式， 状态为聊天会话界面下，仅显示聊天会话列表
    ui->chat_user_list->show();
    ui->con_user_list->hide();
    ui->search_list->hide();
    mode_ = ChatUIMode::CHAT_MODE;
  } else if (state_ == ChatUIMode::CONTACT_MODE) {
    // 非搜索模式， 状态为联系人界面下，仅显示联系人列表
    ui->chat_user_list->hide();
    ui->search_list->hide();
    ui->con_user_list->show();
    mode_ = ChatUIMode::CONTACT_MODE;
  }
}

void ChatDialog::add_lb_group(StateWidget *lb) { lb_list_.push_back(lb); }

void ChatDialog::clear_label_state(StateWidget *lb) {
  for (auto &ele : lb_list_) {
    if (ele == lb) {
      continue;
    }

    ele->ClearState();
  }
}

void ChatDialog::create_connection() {
  connect(ui->chat_user_list, &ChatUserList::sig_loading_chat_user, this,
          &ChatDialog::slot_loading_chat_user);
  connect(ui->con_user_list, &ContactUserList::sig_loading_contact_user, this,
          &ChatDialog::slot_loading_contact_user);

  connect(ui->side_chat_lb, &StateWidget::clicked, this,
          &ChatDialog::slot_side_chat);
  connect(ui->side_contact_lb, &StateWidget::clicked, this,
          &ChatDialog::slot_side_contact);
  connect(ui->search_edit, &QLineEdit::textChanged, this,
          &ChatDialog::slot_text_changed);

  connect(TcpMgr::getinstance().get(), &TcpMgr::sig_recv_friend_apply, this,
          &ChatDialog::slot_handle_friend_apply);

  connect(ui->friend_apply_page, &NewFriendApplyPage::sig_recv_new_friend_apply,
          this, &ChatDialog::slot_show_friend_apply_red_point);

  // 收到对方好友认证后, 将对方(被申请方)加入自己的聊天会话列表
  connect(TcpMgr::getinstance().get(), &TcpMgr::sig_recv_friend_auth, this,
          &ChatDialog::slot_recv_friend_auth);
  //自己处理方对方的好友申请后，将对方（申请方）加入自己的聊天会话列表
  connect(TcpMgr::getinstance().get(), &TcpMgr::sig_friend_apply_rsp, this,
          &ChatDialog::slot_friend_auth_rsp);

  connect(ui->search_list, &SearchList::sig_switch_chat_item, this,
          &ChatDialog::slot_switch_chat_item);
}

void ChatDialog::slot_loading_chat_user() {
  if (b_loading_) { // 正在加载，直接返回
    return;
  }
  // TODO UserManager中加载完所有会话后，限制加载会话信号的发射
  b_loading_ = true;
  qDebug() << "add new data to list.....";
  LoadingDialog *loading_dlg = new LoadingDialog(this);
  loading_dlg->setModal(true);
  loading_dlg->show();
  // add_chat_user_list(); // TODO 该函数应该无用了
  load_more_chat_user();
  // 加载完成后关闭对话框
  loading_dlg->deleteLater();

  b_loading_ = false;

  // 可以通过下面的方法，强行观察加载动画的播放。
  // QTimer::singleShot(500, this, [this, loading_dlg]() {
  //   add_chat_user_list();
  //   loading_dlg->close();
  //   loading_dlg->deleteLater();
  //   b_loading_ = false;
  // });
}

void ChatDialog::slot_loading_contact_user() {
  qDebug() << "slot loading contact user";
  if (b_loading_) {
    return;
  }

  b_loading_ = true;
  LoadingDialog *loadingDialog = new LoadingDialog(this);
  loadingDialog->setModal(true);
  loadingDialog->show();
  qDebug() << "add new data to list.....";
  load_more_contact_user();
  // 加载完成后关闭对话框
  loadingDialog->deleteLater();

  b_loading_ = false;
}
void ChatDialog::slot_side_chat() {
  qDebug() << "receive side chat clicked";
  clear_label_state(ui->side_chat_lb);
  ui->stackedWidget->setCurrentWidget(ui->chat_page);
  state_ = ChatUIMode::CHAT_MODE;
  show_search(false);
}

void ChatDialog::slot_side_contact() {
  qDebug() << "receive side contact clicked";
  clear_label_state(ui->side_contact_lb);
  ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
  state_ = ChatUIMode::CONTACT_MODE;
  show_search(false);
}

void ChatDialog::slot_text_changed(const QString &str) {
  // qDebug()<< "receive slot text changed str is " << str;
  if (!str.isEmpty()) {
    show_search(true);
  }
}

void ChatDialog::slot_handle_friend_apply(
    std::shared_ptr<AddFriendApply> apply) {
  // 判断是否已经申请过了，只显示同一申请者的申请信息
  bool b_already = UserMgr::getinstance()->already_apply(apply->_from_uid);
  if (b_already) {
    return;
  }
  qDebug() << "receive apply friend slot, applyuid is " << apply->_from_uid
           << " name is " << apply->_name << " desc is " << apply->_desc;
  UserMgr::getinstance()->add_apply_list(std::make_shared<ApplyInfo>(apply));
  // 显示红点，表示有未处理信息
  slot_show_friend_apply_red_point();
  // 将好友申请项显示到好友申请页面
  ui->friend_apply_page->add_new_apply(apply);
}

void ChatDialog::slot_show_friend_apply_red_point() {
  ui->side_contact_lb->ShowRedPoint(true);
  ui->con_user_list->ShowRedPoint(true);
}

void ChatDialog::slot_recv_friend_auth(std::shared_ptr<AuthInfo> auth_info) {
  qDebug() << "receive slot_add_auth__friend uid is " << auth_info->_uid
           << " name is " << auth_info->_name << " nick is "
           << auth_info->_nick;

  //判断如果已经是好友则跳过
  auto bfriend = UserMgr::getinstance()->check_friend_by_id(auth_info->_uid);
  if (bfriend) {
    return;
  }
  // 加入到好友列表中
  UserMgr::getinstance()->add_friend(auth_info);

  int randomValue =
      QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
  int str_i = randomValue % strs.size();
  int head_i = randomValue % heads.size();
  int name_i = randomValue % names.size();

  auto *chat_user_wid = new ChatUserWid();
  auto user_info = std::make_shared<UserInfo>(auth_info);
  chat_user_wid->set_user_info(user_info);
  QListWidgetItem *item = new QListWidgetItem;
  // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
  item->setSizeHint(chat_user_wid->sizeHint());
  ui->chat_user_list->insertItem(0, item);
  ui->chat_user_list->setItemWidget(item, chat_user_wid);
  chat_items_added_.insert(auth_info->_uid, item);
}

void ChatDialog::slot_switch_chat_item(std::shared_ptr<SearchInfo> si) {
  qDebug() << "slot jump chat item ";
  auto find_iter = chat_items_added_.find(si->_uid);
  if (find_iter != chat_items_added_.end()) {
    qDebug() << "jump to chat item , uid is " << si->_uid;
    ui->chat_user_list->scrollToItem(find_iter.value());
    ui->side_chat_lb->SetSelected(true);
    set_select_chat_item(si->_uid);
    //更新聊天界面信息
    set_select_chat_page(si->_uid);
    slot_side_chat();
    return;
  }

  //如果没找到，则创建新的插入listwidget

  auto *chat_user_wid = new ChatUserWid();
  auto user_info = std::make_shared<UserInfo>(si);
  chat_user_wid->set_user_info(user_info);
  QListWidgetItem *item = new QListWidgetItem;
  // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
  item->setSizeHint(chat_user_wid->sizeHint());
  ui->chat_user_list->insertItem(0, item);
  ui->chat_user_list->setItemWidget(item, chat_user_wid);

  chat_items_added_.insert(si->_uid, item);

  ui->side_chat_lb->SetSelected(true);
  set_select_chat_item(si->_uid);
  //更新聊天界面信息
  set_select_chat_page(si->_uid);
  slot_side_chat();
}

void ChatDialog::slot_friend_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp) {
  qDebug() << "receive slot_auth_rsp uid is " << auth_rsp->_uid << " name is "
           << auth_rsp->_name << " nick is " << auth_rsp->_nick;

  //判断如果已经是好友则跳过
  auto bfriend = UserMgr::getinstance()->check_friend_by_id(auth_rsp->_uid);
  if (bfriend) {
    return;
  }
  // 加入到好友列表中
  UserMgr::getinstance()->add_friend(auth_rsp);
  // int randomValue =
  //    QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
  // int str_i = randomValue % strs.size();
  // int head_i = randomValue % heads.size();
  // int name_i = randomValue % names.size();

  auto *chat_user_wid = new ChatUserWid();
  auto user_info = std::make_shared<UserInfo>(auth_rsp);
  chat_user_wid->set_user_info(user_info);
  QListWidgetItem *item = new QListWidgetItem;
  // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
  item->setSizeHint(chat_user_wid->sizeHint());
  ui->chat_user_list->insertItem(0, item);
  ui->chat_user_list->setItemWidget(item, chat_user_wid);
  chat_items_added_.insert(auth_rsp->_uid, item);
}