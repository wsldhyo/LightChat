#include "chat_user_wid.hpp"
#include "ui_chatuserwid.h"
#include "user_data.hpp"
ChatUserWid::ChatUserWid(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::ChatUserWid)
{
    ui->setupUi(this);
    SetItemType(ListItemType::CHAT_USER_ITEM);
}

ChatUserWid::~ChatUserWid()
{
    delete ui;
}

void ChatUserWid::set_user_info(std::shared_ptr<UserInfo> user_info)
{
    user_info_ = user_info;
    // 加载图片
    QPixmap pixmap(user_info_->_icon);

    // 设置图片自动缩放
    ui->icon_lb->setPixmap(pixmap.scaled(ui->icon_lb->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    ui->icon_lb->setScaledContents(true);

    ui->user_name_lb->setText(user_info_->_name);
    ui->user_chat_lb->setText(user_info_->_last_msg); // 显示最后一条聊天消息
}

std::shared_ptr<UserInfo> const ChatUserWid::get_user_info()const{
    return user_info_;
}