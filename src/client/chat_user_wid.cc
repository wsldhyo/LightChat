#include "chat_user_wid.hpp"
#include "ui_chatuserwid.h"
#include "user_data.hpp"
ChatUserWid::ChatUserWid(QWidget *parent)
    : ListItemBase(parent), ui(new Ui::ChatUserWid) {
  ui->setupUi(this);
  SetItemType(ListItemType::CHAT_USER_ITEM);
}

ChatUserWid::~ChatUserWid() { delete ui; }

void ChatUserWid::set_friend_info(std::shared_ptr<UserInfo> user_info) {
  friend_info_ = user_info;
  // 加载图片
  QPixmap pixmap(friend_info_->_icon);

  // 设置图片自动缩放
  ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation));
  ui->icon_lb->setScaledContents(true);

  ui->user_name_lb->setText(friend_info_->_name);
  ui->user_chat_lb->setText(friend_info_->_last_msg); // 显示最后一条聊天消息
}

void ChatUserWid::set_friend_info(std::shared_ptr<FriendInfo> user_info) {
  friend_info_ = std::make_shared<UserInfo>(user_info);
  // 加载图片
  QPixmap pixmap(friend_info_->_icon);

  // 设置图片自动缩放
  ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation));
  ui->icon_lb->setScaledContents(true);

  ui->user_name_lb->setText(friend_info_->_name);
  ui->user_chat_lb->setText(friend_info_->_last_msg);
}

std::shared_ptr<UserInfo> const ChatUserWid::get_user_info() const {
  return friend_info_;
}

void ChatUserWid::update_last_msg(
    std::vector<std::shared_ptr<TextChatData>> msgs) {

  QString last_msg = "";
  for (auto &msg : msgs) {
    last_msg = msg->_msg_content;
    friend_info_->_chat_msgs.push_back(msg);
  }

  friend_info_->_last_msg = last_msg;
  ui->user_chat_lb->setText(friend_info_->_last_msg);
}