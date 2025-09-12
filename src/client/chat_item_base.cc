#include "chat_item_base.hpp"
#include "bubble_frame.hpp"
#include <QFont>
#include <QLabel>
#include <QVBoxLayout>
ChatItemBase::ChatItemBase(ChatRole role, QWidget *parent)
    : QWidget(parent), m_role(role) {
  m_pNameLabel = new QLabel();
  m_pNameLabel->setObjectName("chat_user_name");
  m_pNameLabel->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  QFont font("Microsoft YaHei");
  font.setPointSize(9);
  m_pNameLabel->setFont(font);
  m_pNameLabel->setFixedHeight(20);
  m_pIconLabel = new QLabel();
  m_pIconLabel->setScaledContents(true);
  m_pIconLabel->setFixedSize(42, 42);
  m_pBubble = new QWidget();
  QGridLayout *pGLayout = new QGridLayout();
  pGLayout->setVerticalSpacing(3);
  pGLayout->setHorizontalSpacing(3);
  pGLayout->setMargin(3);
  QSpacerItem *pSpacer =
      new QSpacerItem(40, 20, QSizePolicy::Preferred, QSizePolicy::Minimum);
  if (m_role == ChatRole::SELF) {
    m_pNameLabel->setContentsMargins(0, 0, 8, 0);
    m_pNameLabel->setAlignment(Qt::AlignRight);
    pGLayout->addWidget(m_pNameLabel, 0, 1, 1, 1);
    pGLayout->addWidget(m_pIconLabel, 0, 2, 2, 1, Qt::AlignTop);
    QHBoxLayout *cellLayout = new QHBoxLayout();
    // cellLayout里的Spacer在气泡宽度小于namelabel宽度时，通过ChatItemBase::set_spacer_width调整宽度，将气泡压向头像框
    cellLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Fixed,
                                              QSizePolicy::Minimum)); // 左边距
    cellLayout->addWidget(m_pBubble);

    pGLayout->addItem(pSpacer, 1, 0, 1, 1);
    pGLayout->addLayout(cellLayout, 1, 1, 1, 1);
    pGLayout->setColumnStretch(0, 2);
    pGLayout->setColumnStretch(1, 8);
  } else {
    m_pNameLabel->setContentsMargins(8, 0, 0, 0);
    m_pNameLabel->setAlignment(Qt::AlignLeft);
    pGLayout->addWidget(m_pIconLabel, 0, 0, 2, 1, Qt::AlignTop);
    pGLayout->addWidget(m_pNameLabel, 0, 1, 1, 1);

    QHBoxLayout *cellLayout = new QHBoxLayout();
    cellLayout->addWidget(m_pBubble);
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

void ChatItemBase::setUserName(const QString &name) {
  m_pNameLabel->setText(name);
  m_pNameLabel->adjustSize(); // 根据内容调整大小
}

void ChatItemBase::setUserIcon(const QPixmap &icon) {
  m_pIconLabel->setPixmap(icon);
}

void ChatItemBase::setWidget(QWidget *w) {
  QGridLayout *pGLayout = (qobject_cast<QGridLayout *>)(this->layout());
  pGLayout->replaceWidget(m_pBubble, w);
  delete m_pBubble;
  m_pBubble = w;
}

int ChatItemBase::name_width() const { return m_pNameLabel->width(); }

void ChatItemBase::set_spacer_width(int width) {
  QGridLayout *layout = static_cast<QGridLayout *>(this->layout());
  if (!layout)
    return;

  QHBoxLayout *hLayout =
      static_cast<QHBoxLayout *>(layout->itemAtPosition(1, 1)->layout());
  if (!hLayout || hLayout->count() == 0)
    return;

  QSpacerItem *spacer = nullptr;

  if (m_role == ChatRole::SELF) {
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
