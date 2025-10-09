#include "friend_info_page.hpp"
#include "ui_friendinfopage.h"
#include "user_data.hpp"
#include <QDebug>
FriendInfoPage::FriendInfoPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::FriendInfoPage), user_info_(nullptr) {
  ui->setupUi(this);
  ui->msg_chat->SetState("normal", "hover", "press");
  ui->video_chat->SetState("normal", "hover", "press");
  ui->voice_chat->SetState("normal", "hover", "press");
}

FriendInfoPage::~FriendInfoPage() { delete ui; }

void FriendInfoPage::SetInfo(std::shared_ptr<UserInfo> user_info) {
  user_info_ = user_info;
  // 加载图片
  QPixmap pixmap(user_info->_icon);

  // 设置图片自动缩放
  ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation));
  ui->icon_lb->setScaledContents(true);

  ui->name_lb->setText(user_info->_name);
  ui->nick_lb->setText(user_info->_nick);
  ui->bak_lb->setText(user_info->_nick);
}

void FriendInfoPage::on_msg_chat_clicked() {
  qDebug() << "msg chat btn clicked";
  emit sig_jump_chat_item(user_info_);
}
