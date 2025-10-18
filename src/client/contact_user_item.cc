#include "contact_user_item.hpp"
#include "ui_contactuseritem.h"
#include "user_data.hpp"

ContactUserItem::ContactUserItem(QWidget *parent)
    : ListItemBase(parent), ui(new Ui::ContactUserItem) {
  ui->setupUi(this);
  SetItemType(ListItemType::CONTACT_USER_ITEM);

  ui->red_point->raise(); // 保证红点显示在最顶层
  show_red_point(false);  // 默认不显示红点
}

ContactUserItem::~ContactUserItem() { delete ui; }

QSize ContactUserItem::sizeHint() const {
  return QSize(250, 70); // 固定联系人条目的高度
}

// 设置联系人信息（AuthInfo）
void ContactUserItem::set_info(std::shared_ptr<AuthInfo> auth_info) {
  friend_info_ = std::make_shared<UserInfo>(auth_info);
  set_friend_info();
}

// 设置联系人信息（uid、name、icon）
void ContactUserItem::set_info(int uid, QString name, QString icon) {
  friend_info_ = std::make_shared<UserInfo>(uid, name, icon);
  set_friend_info();
}

// 设置联系人信息（AuthRsp）
void ContactUserItem::set_info(std::shared_ptr<AuthRsp> auth_rsp) {
  friend_info_ = std::make_shared<UserInfo>(auth_rsp);
  set_friend_info();
}

// 控制右上角红点显隐
void ContactUserItem::show_red_point(bool show) {
  if (show) {
    ui->red_point->show();
  } else {
    ui->red_point->hide();
  }
}

std::shared_ptr<UserInfo> const ContactUserItem::get_user_info() const {
  return friend_info_;
}

void ContactUserItem::set_friend_info() {
  QPixmap pixmap(friend_info_->icon_);
  ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation));
  ui->icon_lb->setScaledContents(true);

  ui->user_name_lb->setText(friend_info_->name_);
}