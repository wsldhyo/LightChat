#include "bubble_frame.hpp"
#include <QDebug>
#include <QHBoxLayout>
#include <QPainter>
BubbleFrame::BubbleFrame(ChatRole role, QWidget *parent)
    : QFrame(parent), m_role(role), m_margin(3) {
  m_pHLayout = new QHBoxLayout();
  if (m_role == ChatRole::SELF)
    m_pHLayout->setContentsMargins(m_margin, m_margin,
                                   BUBBLE_TRIANGLE_WIDTH + m_margin, m_margin);
  else
    m_pHLayout->setContentsMargins(BUBBLE_TRIANGLE_WIDTH + m_margin, m_margin,
                                   m_margin, m_margin);

  this->setLayout(m_pHLayout);
  this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

void BubbleFrame::setMargin(int margin) {
  Q_UNUSED(margin);
  // m_margin = margin;
}

void BubbleFrame::setWidget(QWidget *w) {
  if (m_pHLayout->count() > 0)
    return;
  else {
    m_pHLayout->addWidget(w);
  }
}
void BubbleFrame::paintEvent(QPaintEvent *e) {
    QPainter painter(this);
    painter.setPen(Qt::NoPen);

    // 获取 text_edit 的几何
    QRect editRect;
    if (m_pHLayout && m_pHLayout->count() > 0) {
        if (auto w = m_pHLayout->itemAt(0)->widget()) {
            editRect = w->geometry();
        }
    }

    if (!editRect.isValid()) {
        return QFrame::paintEvent(e);
    }

    // 扩展气泡矩形，但不覆盖小三角
    QRect bubbleRect = editRect.adjusted(-m_margin, -m_margin, m_margin, m_margin);

    if (m_role == ChatRole::OTHERS) {
        QColor bk_color(Qt::white);
        painter.setBrush(QBrush(bk_color));
        // 左侧小三角占用空间，所以矩形向右移动
        QRect rectForBubble = bubbleRect;
        rectForBubble.setLeft(bubbleRect.left() + BUBBLE_TRIANGLE_WIDTH);
        painter.drawRoundedRect(rectForBubble, 5, 5);

        // 小三角（左边）
        QPointF points[3] = {
            QPointF(rectForBubble.left() - BUBBLE_TRIANGLE_WIDTH, rectForBubble.top() + 12),
            QPointF(rectForBubble.left(), rectForBubble.top() + 12),
            QPointF(rectForBubble.left(), rectForBubble.top() + 12 + BUBBLE_TRIANGLE_WIDTH),
        };
        painter.drawPolygon(points, 3);
    } else {
        QColor bk_color(158, 234, 106);
        painter.setBrush(QBrush(bk_color));
        // 右侧小三角占用空间，矩形缩短右边
        QRect rectForBubble = bubbleRect;
        rectForBubble.setRight(bubbleRect.right() - BUBBLE_TRIANGLE_WIDTH);
        painter.drawRoundedRect(rectForBubble, 5, 5);

        // 小三角（右边）
        QPointF points[3] = {
            QPointF(rectForBubble.right(), rectForBubble.top() + 12),
            QPointF(rectForBubble.right() + BUBBLE_TRIANGLE_WIDTH, rectForBubble.top() + 12 + BUBBLE_TRIANGLE_WIDTH / 2.0),
            QPointF(rectForBubble.right(), rectForBubble.top() + 12 + BUBBLE_TRIANGLE_WIDTH),
        };
        painter.drawPolygon(points, 3);
    }

    QFrame::paintEvent(e);
}


// void BubbleFrame::paintEvent(QPaintEvent *e) {
//     QPainter painter(this);
//     painter.setPen(Qt::NoPen);
//
//     // 画气泡之前，先画三角形
//     if (m_role == ChatRole::OTHERS) {
//         QColor bk_color(Qt::white);
//         painter.setBrush(QBrush(bk_color));
//
//         // 画小三角
//         QPointF points[3] = {
//             QPointF(BUBBLE_TRIANGLE_WIDTH, 12),
//             QPointF(BUBBLE_TRIANGLE_WIDTH, 10 + BUBBLE_TRIANGLE_WIDTH + 2),
//             QPointF(BUBBLE_TRIANGLE_WIDTH - BUBBLE_TRIANGLE_WIDTH,
//                     10 + BUBBLE_TRIANGLE_WIDTH - BUBBLE_TRIANGLE_WIDTH
//                     / 2.0),
//         };
//         painter.drawPolygon(points, 3);
//
//         // 根据三角形的位置绘制矩形
//         QRect bk_rect =
//             QRect(BUBBLE_TRIANGLE_WIDTH, 0, this->width() -
//             BUBBLE_TRIANGLE_WIDTH, this->height());
//         painter.drawRoundedRect(bk_rect, 5, 5);
//     } else {
//         QColor bk_color(158, 234, 106);
//         painter.setBrush(QBrush(bk_color));
//
//         // 画三角
//         QPointF points[3] = {
//             QPointF(this->width() - BUBBLE_TRIANGLE_WIDTH, 12),
//             QPointF(this->width() - BUBBLE_TRIANGLE_WIDTH, 12 +
//             BUBBLE_TRIANGLE_WIDTH + 2), QPointF(this->width(), 10 +
//             BUBBLE_TRIANGLE_WIDTH - BUBBLE_TRIANGLE_WIDTH / 2.0),
//         };
//         painter.drawPolygon(points, 3);
//
//         // 根据三角形的位置绘制矩形
//         QRect bk_rect = QRect(0, 0, this->width() - BUBBLE_TRIANGLE_WIDTH,
//         this->height()); painter.drawRoundedRect(bk_rect, 5, 5);
//     }
//
//     return QFrame::paintEvent(e);
// }
