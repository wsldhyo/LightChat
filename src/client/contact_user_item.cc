#include "contact_user_item.hpp"
#include "ui_contactuseritem.h"
#include "user_data.hpp"

ContactUserItem::ContactUserItem(QWidget *parent)
    : ListItemBase(parent), ui(new Ui::ContactUserItem) {
  ui->setupUi(this);
  SetItemType(ListItemType::CONTACT_USER_ITEM);

  ui->red_point->raise(); // 保证红点显示在最顶层
  ShowRedPoint(false);    // 默认不显示红点
}

ContactUserItem::~ContactUserItem() { delete ui; }

QSize ContactUserItem::sizeHint() const {
  return QSize(250, 70); // 固定联系人条目的高度
}

// 设置联系人信息（AuthInfo）
void ContactUserItem::SetInfo(std::shared_ptr<AuthInfo> auth_info) {
  _info = std::make_shared<UserInfo>(auth_info);

  // 加载头像并缩放
  QPixmap pixmap(_info->_icon);
  ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation));
  ui->icon_lb->setScaledContents(true);

  ui->user_name_lb->setText(_info->_name);
}

// 设置联系人信息（uid、name、icon）
void ContactUserItem::SetInfo(int uid, QString name, QString icon) {
  _info = std::make_shared<UserInfo>(uid, name, icon);

  QPixmap pixmap(_info->_icon);
  ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation));
  ui->icon_lb->setScaledContents(true);

  ui->user_name_lb->setText(_info->_name);
}

// 设置联系人信息（AuthRsp）
void ContactUserItem::SetInfo(std::shared_ptr<AuthRsp> auth_rsp) {
  _info = std::make_shared<UserInfo>(auth_rsp);

  QPixmap pixmap(_info->_icon);
  ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation));
  ui->icon_lb->setScaledContents(true);

  ui->user_name_lb->setText(_info->_name);
}

// 控制右上角红点显隐
void ContactUserItem::ShowRedPoint(bool show) {
  if (show) {
    ui->red_point->show();
  } else {
    ui->red_point->hide();
  }
}

std::shared_ptr<UserInfo> const ContactUserItem::get_user_info() const {
  return _info;
}
