#include "picture_bubble.hpp"
#include "client_constant.hpp"
#include <QHBoxLayout>
#include <QLabel>

PictureBubble::PictureBubble(const QPixmap &picture, ChatRole role,
                             QWidget *parent)
    : BubbleFrame(role, parent) {
  QLabel *lb = new QLabel();
  lb->setScaledContents(true);
  QPixmap pix =
      picture.scaled(QSize(PIC_MAX_WIDTH, PIC_MAX_HEIGHT), Qt::KeepAspectRatio);
  lb->setPixmap(pix);
  this->set_widget(lb);

  int left_margin = this->layout()->contentsMargins().left();
  int right_margin = this->layout()->contentsMargins().right();
  int v_margin = this->layout()->contentsMargins().bottom();
  // 设置宽高
  setFixedSize(pix.width() + left_margin + right_margin,
               pix.height() + v_margin * 2);
}