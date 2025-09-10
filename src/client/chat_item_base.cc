#include "chat_item_base.hpp"
#include "bubble_frame.hpp"
#include <QFont>
#include <QLabel>
#include <QVBoxLayout>
ChatItemBase::ChatItemBase(ChatRole role, QWidget *parent)
    : QWidget(parent), role_(role) {
  name_label_ = new QLabel();
  name_label_->setObjectName("chat_user_name");
  QFont font("Microsoft YaHei");
  font.setPointSize(9);
  name_label_->setFont(font);
  name_label_->setFixedHeight(20);
  icon_label_ = new QLabel();
  icon_label_->setScaledContents(true);
  icon_label_->setFixedSize(42, 42);
  bubble_ = new QWidget();

  QGridLayout *pGLayout = new QGridLayout();
  pGLayout->setVerticalSpacing(3);
  pGLayout->setHorizontalSpacing(3);
  pGLayout->setMargin(3);

  // 弹簧有最小大小，保证拉伸时具有一定空白
  QSpacerItem *pSpacer =
      new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
  if (role_ == ChatRole::SELF) {

    // 如果消息是本人发出的， 则将弹簧放到左边，消息框放到中间，头像框放到右边, 
    name_label_->setContentsMargins(0, 0, 8, 0);  // name_label_到头像框有8像素距离
    name_label_->setAlignment(Qt::AlignRight);
    pGLayout->addWidget(name_label_, 0, 1, 1, 1);
    pGLayout->addWidget(icon_label_, 0, 2, 2, 1, Qt::AlignTop);
    pGLayout->addItem(pSpacer, 1, 0, 1, 1);
    pGLayout->addWidget(bubble_, 1, 1, 1, 1);
    pGLayout->setColumnStretch(0, 2);
    pGLayout->setColumnStretch(1, 3);
  } else {
    // 如果消息是他人发出的， 则将头像框放到左边，消息框放到中间，弹簧放到右边
    name_label_->setContentsMargins(8, 0, 0, 0);
    name_label_->setAlignment(Qt::AlignLeft);
    pGLayout->addWidget(icon_label_, 0, 0, 2, 1, Qt::AlignTop);
    pGLayout->addWidget(name_label_, 0, 1, 1, 1);
    pGLayout->addWidget(bubble_, 1, 1, 1, 1);
    pGLayout->addItem(pSpacer, 2, 2, 1, 1);
    pGLayout->setColumnStretch(1, 3);
    pGLayout->setColumnStretch(2, 2);
  }
  this->setLayout(pGLayout);
}

void ChatItemBase::setUserName(const QString &name) {
  name_label_->setText(name);
}

void ChatItemBase::setUserIcon(const QPixmap &icon) {
  icon_label_->setPixmap(icon);
}

void ChatItemBase::setWidget(QWidget *w) {
  QGridLayout *pGLayout = (qobject_cast<QGridLayout *>)(this->layout());
  pGLayout->replaceWidget(bubble_, w);
  delete bubble_;   // 从网格布局中移除，不受对象树管理，需要显式delete，避免内存泄露
  bubble_ = w;
}