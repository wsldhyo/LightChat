#include "new_friend_apply_item.hpp"
#include "ui_newfriendapplyitem.h"
#include "user_data.hpp"

NewFriendApplyItem::NewFriendApplyItem(QWidget *parent)
    : ListItemBase(parent), added_(false), ui(new Ui::NewFriendApplyItem) {
  ui->setupUi(this);
  SetItemType(ListItemType::APPLY_FRIEND_ITEM);

  // 设置按钮状态图片
  ui->addBtn->SetState("normal", "hover", "press");
  ui->addBtn->hide(); // 初始隐藏

  // 点击“添加好友”按钮时发出信号
  connect(ui->addBtn, &ClickedBtn::clicked,
          [this]() { emit this->sig_auth_friend(apply_info_); });
}

NewFriendApplyItem::~NewFriendApplyItem() { delete ui; }

void NewFriendApplyItem::set_info(std::shared_ptr<ApplyInfo> apply_info) {
  apply_info_ = apply_info;

  // 设置头像，并自适应 QLabel
  QPixmap pixmap(apply_info_->_icon);
  ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation));
  ui->icon_lb->setScaledContents(true);

  // 设置昵称和描述
  ui->user_name_lb->setText(apply_info_->_name);
  ui->user_chat_lb->setText(apply_info_->_desc);
}

void NewFriendApplyItem::show_add_btn(bool bshow) {
  if (bshow) {
    ui->addBtn->show();
    ui->already_add_lb->hide();
    added_ = false;
  } else {
    qDebug() << "show added label";
    ui->addBtn->hide();
    ui->already_add_lb->show();
    added_ = true;
  }
}

int NewFriendApplyItem::get_uid() { return apply_info_->_uid; }