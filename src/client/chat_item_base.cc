#include "chat_item_base.hpp"
#include "bubble_frame.hpp"
#include <QFont>
#include <QLabel>
#include <QVBoxLayout>
ChatItemBase::ChatItemBase(ChatRole role, QWidget *parent)
    : QWidget(parent), role_(role) {
  name_label_ = new QLabel();
  name_label_->setObjectName("chat_user_name");
  name_label_->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
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
  QSpacerItem *pSpacer =
      new QSpacerItem(40, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);
  if (role_ == ChatRole::SELF) {
    name_label_->setContentsMargins(0, 0, 8, 0);
    name_label_->setAlignment(Qt::AlignRight);
    pGLayout->addWidget(name_label_, 0, 1, 1, 1);
    pGLayout->addWidget(icon_label_, 0, 2, 2, 1, Qt::AlignTop);
    QHBoxLayout *cellLayout = new QHBoxLayout();
    // cellLayout里的Spacer在气泡宽度小于namelabel宽度时，通过ChatItemBase::set_spacer_width调整宽度，将气泡压向头像框
    cellLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Fixed,
                                              QSizePolicy::Minimum)); // 左边距
    cellLayout->addWidget(bubble_);

    pGLayout->addItem(pSpacer, 1, 0, 1, 1);
    pGLayout->addLayout(cellLayout, 1, 1, 1, 1);
    pGLayout->setColumnStretch(0, 2);
    pGLayout->setColumnStretch(1, 8);
  } else {
    name_label_->setContentsMargins(8, 0, 0, 0);
    name_label_->setAlignment(Qt::AlignLeft);
    pGLayout->addWidget(icon_label_, 0, 0, 2, 1, Qt::AlignTop);
    pGLayout->addWidget(name_label_, 0, 1, 1, 1);

    QHBoxLayout *cellLayout = new QHBoxLayout();
    cellLayout->addWidget(bubble_);
    // cellLayout里的Spacer在气泡宽度小于namelabel宽度时，通过ChatItemBase::set_spacer_width调整宽度，将气泡压向头像框
    cellLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Fixed,
                                              QSizePolicy::Minimum)); // 右边距

    pGLayout->addLayout(cellLayout, 1, 1, 1, 1);
    pGLayout->addItem(pSpacer, 2, 2, 1, 1);
    pGLayout->setColumnStretch(1, 8);
    pGLayout->setColumnStretch(2, 2);
  }
  this->setLayout(pGLayout);
}

void ChatItemBase::set_user_name(const QString &name) {
  name_label_->setText(name);
  name_label_->adjustSize(); // 根据内容调整大小
}

void ChatItemBase::set_user_icon(const QPixmap &icon) {
  icon_label_->setPixmap(icon);
}

void ChatItemBase::set_widget(QWidget *w) {
  QGridLayout *pGLayout = (qobject_cast<QGridLayout *>)(this->layout());
  pGLayout->replaceWidget(bubble_, w);
  delete bubble_;
  bubble_ = w;
}

int ChatItemBase::name_width() const { return name_label_->width(); }

void ChatItemBase::set_spacer_width(int width) {
  QGridLayout *layout = static_cast<QGridLayout *>(this->layout());
  if (!layout)
    return;

  QHBoxLayout *hLayout =
      static_cast<QHBoxLayout *>(layout->itemAtPosition(1, 1)->layout());
  if (!hLayout || hLayout->count() == 0)
    return;

  QSpacerItem *spacer = nullptr;

  if (role_ == ChatRole::SELF) {
    // 左边距
    spacer = hLayout->itemAt(0)->spacerItem();
  } else {
    // 右边距
    spacer = hLayout->itemAt(1)->spacerItem();
  }

  if (!spacer)
    return;

  spacer->changeSize(width, 0, QSizePolicy::Fixed, QSizePolicy::Minimum);
  layout->invalidate();
}
