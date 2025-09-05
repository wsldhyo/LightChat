#include "bubble_frame.hpp"
#include <QDebug>
#include <QHBoxLayout>
#include <QPainter>

//连接头像框与气泡框的小三角的宽度
const int WIDTH_TRIANGLE = 8;

BubbleFrame::BubbleFrame(ChatRole role, QWidget *parent)
    : QFrame(parent), role_(role), margin_(3) {
  hlayout_ = new QHBoxLayout();
  if (role_ == ChatRole::SELF) {

    // 自己发送的消息，头像框在右边，小三角也在消息右边，
    // 内容到右边距为margin_ + WIDTH_TRIANGLE
    hlayout_->setContentsMargins(margin_, margin_, WIDTH_TRIANGLE + margin_,
                                 margin_);
  } else {

    // 他人发送的消息，头像框在左边，小三角也在消息主题左边，
    // 内容到右边距为margin_ + WIDTH_TRIANGLE
    hlayout_->setContentsMargins(WIDTH_TRIANGLE + margin_, margin_, margin_,
                                 margin_);
  }

  this->setLayout(hlayout_);
}

void BubbleFrame::setMargin(int margin) {
  Q_UNUSED(margin);
  // margin_ = margin;
}

void BubbleFrame::setWidget(QWidget *w) {
  if (hlayout_->count() > 0)
    return;
  else {
    hlayout_->addWidget(w);
  }
}
// 重写paintEvent，绘制气泡聊天框
void BubbleFrame::paintEvent(QPaintEvent *e) {
  QPainter painter(this);
  painter.setPen(Qt::NoPen); // 不带边框

  if (role_ == ChatRole::OTHERS) {
    QColor bk_color(Qt::white); // 他人消息，背景白色
    painter.setBrush(QBrush(bk_color));

    //画气泡， 左边预留小三角形的宽度，右边占满父Widget
    QRect bk_rect = QRect(WIDTH_TRIANGLE, 0, this->width() - WIDTH_TRIANGLE,
                          this->height());
    painter.drawRoundedRect(bk_rect, 5, 5);
    //画小三角
    QPointF points[3] = {
        QPointF(bk_rect.x(), 12),
        QPointF(bk_rect.x(), 10 + WIDTH_TRIANGLE + 2),
        QPointF(bk_rect.x() - WIDTH_TRIANGLE,
                10 + WIDTH_TRIANGLE - WIDTH_TRIANGLE / 2.0),
    };
    painter.drawPolygon(points, 3);
  } else {
    QColor bk_color(158, 234, 106); // 自己消息，背景绿色
    painter.setBrush(QBrush(bk_color));
    //画气泡
    QRect bk_rect = QRect(0, 0, this->width() - WIDTH_TRIANGLE, this->height());
    painter.drawRoundedRect(bk_rect, 5, 5);
    //画三角
    QPointF points[3] = {
        QPointF(bk_rect.x() + bk_rect.width(), 12),
        QPointF(bk_rect.x() + bk_rect.width(), 12 + WIDTH_TRIANGLE + 2),
        QPointF(bk_rect.x() + bk_rect.width() + WIDTH_TRIANGLE,
                10 + WIDTH_TRIANGLE - WIDTH_TRIANGLE / 2.0),
    };
    painter.drawPolygon(points, 3);
  }

  return QFrame::paintEvent(e);
}
