#include "chat_dlg.hpp"
#include <QAction>
#include <QDebug>
ChatDlg::ChatDlg(QWidget *_parent /*nullptr*/)
    : QDialog(_parent), ui_(new Ui::ChatDialog), mode_(ChatUiMode::CHAT_MODE),
      state_(ChatUiMode::CHAT_MODE), b_loading(false) {
  ui_->setupUi(this);
  ui_->add_btn->set_state("normal", "hover", "press");
  ui_->search_edit->set_max_length(15);

  // 搜索框前面的放大镜图标
  search_action_ = new QAction(ui_->search_edit);
  search_action_->setIcon(QIcon(":/icons/search.png"));
  ui_->search_edit->addAction(search_action_, QLineEdit::LeadingPosition);
  ui_->search_edit->setPlaceholderText(QStringLiteral("搜索"));

  // 搜索框后面的×图标，用于清除搜索框文本内容
  clear_action_ = new QAction(ui_->search_edit);
  clear_action_->setIcon(QIcon(":/icons/close_transparent.png"));
  // 初始时不显示清除图标
  // 将清除动作添加到LineEdit的末尾位置
  ui_->search_edit->addAction(clear_action_, QLineEdit::TrailingPosition);

  create_connection();
  show_search(false); // 默认隐藏
}

ChatDlg::~ChatDlg() { delete ui_; }

void ChatDlg::create_connection() {
  // 当用户输入文本时，显示初始被隐藏的搜索框删除图标
  connect(
      ui_->search_edit, &QLineEdit::textChanged, [this](const QString &text) {
        if (!text.isEmpty()) {
          clear_action_->setIcon(QIcon(":/icons/close_search.png"));
        } else {
          clear_action_->setIcon(QIcon(
              ":/icons/close_transparent.png")); // 文本为空时，切换回透明图标
        }
      });

  // 点击搜索框删除图标时，清除文本
  connect(clear_action_, &QAction::triggered, [this]() {
    ui_->search_edit->clear();
    clear_action_->setIcon(
        QIcon(":/icons/close_transparent.png")); // 清除文本后，切换回透明图标
    ui_->search_edit->clearFocus();
    // 清除按钮被按下则不显示搜索框
    show_search(false);
  });
}

void ChatDlg::show_search(bool _bsearch) {
  if (_bsearch) {
    ui_->chat_user_list->hide();
    ui_->con_user_list->hide();
    ui_->search_list->show();
    mode_ = ChatUiMode::SEARCH_MODE;
  } else if (state_ == ChatUiMode::CHAT_MODE) {
    ui_->chat_user_list->show();
    ui_->con_user_list->hide();
    ui_->search_list->hide();
    mode_ = ChatUiMode::CHAT_MODE;
    //ui_->search_list->CloseFindDlg();
    ui_->search_edit->clear();
    ui_->search_edit->clearFocus();
  } else if (state_ == ChatUiMode::CONTRACT_MODE) {
    ui_->chat_user_list->hide();
    ui_->search_list->hide();
    ui_->con_user_list->show();
    mode_ = ChatUiMode::CONTRACT_MODE;
    //ui_->search_list->CloseFindDlg();
    ui_->search_edit->clear();
    ui_->search_edit->clearFocus();
  }
}