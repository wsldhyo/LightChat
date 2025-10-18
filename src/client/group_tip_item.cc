#include "group_tip_item.hpp"
#include "ui_grouptipitem.h"

GroupTipItem::GroupTipItem(QWidget *parent)
    : ListItemBase(parent), tip_(""), ui(new Ui::GroupTipItem) {
  ui->setupUi(this);
  // 设置为分组提示类型（不可点击）
  SetItemType(ListItemType::GROUP_TIP_ITEM);
}

GroupTipItem::~GroupTipItem() { delete ui; }

QSize GroupTipItem::sizeHint() const {
  return QSize(250, 25); // 固定分组提示条目的高度
}

void GroupTipItem::set_group_tip(QString str) {
  ui->label->setText(str); // 设置 UI 标签文本
}
