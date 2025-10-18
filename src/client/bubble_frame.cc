#include "bubble_frame.hpp"
#include <QDebug>
#include <QHBoxLayout>
#include <QPainter>

//连接头像框与气泡框的小三角的宽度
BubbleFrame::BubbleFrame(ChatRole role, QWidget *parent)
    : QFrame(parent), role_(role), margin_(3) {
  h_layout_ = new QHBoxLayout();
  if (role_ == ChatRole::SELF)
    h_layout_->setContentsMargins(margin_, margin_,
                                   BUBBLE_TRIANGLE_WIDTH + margin_, margin_);
  else
    h_layout_->setContentsMargins(BUBBLE_TRIANGLE_WIDTH + margin_, margin_,
                                   margin_, margin_);

  this->setLayout(h_layout_);
}

void BubbleFrame::set_margin(int margin) {
  Q_UNUSED(margin);
  // margin_ = margin;
}

void BubbleFrame::set_widget(QWidget *w) {
  if (h_layout_->count() > 0)
    return;
  else {
    h_layout_->addWidget(w);
  }
}

void BubbleFrame::paintEvent(QPaintEvent *e) {
  QPainter painter(this);
  painter.setPen(Qt::NoPen);

  if (role_ == ChatRole::OTHERS) {
    //画气泡
    QColor bk_color(Qt::white);
    painter.setBrush(QBrush(bk_color));
    QRect bk_rect =
        QRect(BUBBLE_TRIANGLE_WIDTH, 0, this->width() - BUBBLE_TRIANGLE_WIDTH,
              this->height());
    painter.drawRoundedRect(bk_rect, 5, 5);
    //画小三角
    QPointF points[3] = {
        QPointF(bk_rect.x(), 12),
        QPointF(bk_rect.x(), 10 + BUBBLE_TRIANGLE_WIDTH + 2),
        QPointF(bk_rect.x() - BUBBLE_TRIANGLE_WIDTH,
                10 + BUBBLE_TRIANGLE_WIDTH - BUBBLE_TRIANGLE_WIDTH / 2.0),
    };
    painter.drawPolygon(points, 3);
  } else {
    QColor bk_color(158, 234, 106);
    painter.setBrush(QBrush(bk_color));
    //画气泡
    QRect bk_rect =
        QRect(0, 0, this->width() - BUBBLE_TRIANGLE_WIDTH, this->height());
    painter.drawRoundedRect(bk_rect, 5, 5);
    //画三角
    QPointF points[3] = {
        QPointF(bk_rect.x() + bk_rect.width(), 12),
        QPointF(bk_rect.x() + bk_rect.width(), 12 + BUBBLE_TRIANGLE_WIDTH + 2),
        QPointF(bk_rect.x() + bk_rect.width() + BUBBLE_TRIANGLE_WIDTH,
                10 + BUBBLE_TRIANGLE_WIDTH - BUBBLE_TRIANGLE_WIDTH / 2.0),
    };
    painter.drawPolygon(points, 3);
  }

  return QFrame::paintEvent(e);
}
