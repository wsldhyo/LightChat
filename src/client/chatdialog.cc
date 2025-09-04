#include "chatdialog.hpp"
#include "chatuserwid.hpp"
#include "loadingdialog.hpp"
#include "ui_chatdialog.h"
#include <QAction>
#include <QDebug>
#include <QRandomGenerator>
#include <future>
ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::ChatDialog), mode_(ChatUIMode::CHAT_MODE),
      state_(ChatUIMode::CHAT_MODE), b_loading_(false)

{
  ui->setupUi(this);
  ui->add_btn->SetState("normal", "hover", "press");
  set_search_edit();
  add_chat_user_list();
  create_connection();
}

ChatDialog::~ChatDialog() { delete ui; }

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

std::vector<QString> strs = {
    "hello world !", "nice to meet u", "New year，new life",
    "You have to love yourself",
    "My love is written in the wind ever since the whole world is you"};

std::vector<QString> heads = {":/icons/head_1.jpg", ":/icons/head_2.jpg",
                              ":/icons/head_3.jpg", ":/icons/head_4.jpg",
                              ":/icons/head_5.jpg"};

std::vector<QString> names = {"llfc", "zack",   "golang", "cpp",
                              "java", "nodejs", "python", "rust"};
void ChatDialog::add_chat_user_list() {
  // 创建QListWidgetItem，并设置自定义的widget
  for (int i = 0; i < 13; i++) {
    int randomValue =
        QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    int str_i = randomValue % strs.size();
    int head_i = randomValue % heads.size();
    int name_i = randomValue % names.size();

    auto *chat_user_wid = new ChatUserWid();
    chat_user_wid->SetInfo(names[name_i], heads[head_i], strs[str_i]);
    QListWidgetItem *item = new QListWidgetItem;
    // qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    // 将item加入聊天会话列表，并将item设置为自定义的widget
    ui->chat_user_list->addItem(item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
  }
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

void ChatDialog::create_connection() {
  connect(ui->chat_user_list, &ChatUserList::sig_loading_chat_user, this,
          &ChatDialog::slot_loading_chat_user);
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
  add_chat_user_list();
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