#include "friend_label.hpp"
#include "clicked_label.hpp"
#include "ui_friendlabel.h"
#include <QDebug>
FriendLabel::FriendLabel(QWidget *parent)
    : QFrame(parent), ui(new Ui::FriendLabel) {
  ui->setupUi(this);
  ui->close_lb->set_state("normal", "hover", "pressed", "selected_normal",
                         "selected_hover", "selected_pressed");
  connect(ui->close_lb, &ClickedLabel::clicked, this, &FriendLabel::slot_close);
}

FriendLabel::~FriendLabel() { delete ui; }

void FriendLabel::set_text(QString text) {
  text_ = text;
  ui->tip_lb->setText(text_);
  ui->tip_lb->adjustSize();

  QFontMetrics fontMetrics(ui->tip_lb->font()); // 获取QLabel控件的字体信息
  auto textWidth = fontMetrics.horizontalAdvance(ui->tip_lb->text()); // 获取文本的宽度
  auto textHeight = fontMetrics.height(); // 获取文本的高度

  qDebug() << " ui->tip_lb.width() is " << ui->tip_lb->width();
  qDebug() << " ui->close_lb->width() is " << ui->close_lb->width();
  qDebug() << " textWidth is " << textWidth;
  this->setFixedWidth(ui->tip_lb->width() + ui->close_lb->width() + 5);
  this->setFixedHeight(textHeight + 2);
  qDebug() << "  this->setFixedHeight " << this->height();
  width_ = this->width();
  height_ = this->height();
}

int FriendLabel::width() { return width_; }

int FriendLabel::height() { return height_; }

QString FriendLabel::text() { return text_; }

void FriendLabel::slot_close() { emit sig_close(text_); }