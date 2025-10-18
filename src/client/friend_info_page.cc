#include "friend_info_page.hpp"
#include "ui_friendinfopage.h"
#include "user_data.hpp"
#include <QDebug>
FriendInfoPage::FriendInfoPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::FriendInfoPage), friend_info_(nullptr) {
  ui->setupUi(this);
  ui->msg_chat->set_state("normal", "hover", "press");
  ui->video_chat->set_state("normal", "hover", "press");
  ui->voice_chat->set_state("normal", "hover", "press");
}

FriendInfoPage::~FriendInfoPage() { delete ui; }

void FriendInfoPage::set_friend_info(std::shared_ptr<UserInfo> friend_info) {
  friend_info_ = friend_info;
  // 加载图片
  QPixmap pixmap(friend_info->icon_);

  // 设置图片自动缩放
  ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation));
  ui->icon_lb->setScaledContents(true);

  ui->name_lb->setText(friend_info->name_);
  ui->nick_lb->setText(friend_info->nick_);
  ui->bak_lb->setText(friend_info->nick_);
}

void FriendInfoPage::on_msg_chat_clicked() {
  qDebug() << "msg chat btn clicked";
  emit sig_jump_chat_item(friend_info_);
}
