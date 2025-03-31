#include "customize_edit.hpp"

CustomizeEdit::CustomizeEdit(QWidget *_parent /*nullptr*/)
    : QLineEdit(_parent) {
      create_connection();
    }

void CustomizeEdit::set_max_length(int _max_len) { max_len_ = _max_len; }

void CustomizeEdit::focusOutEvent(QFocusEvent *event) {
  // 执行失去焦点时的处理逻辑
  // qDebug() << "CustomizeEdit focusout";
  // 调用基类的focusOutEvent()方法，保证基类的行为得到执行
  QLineEdit::focusOutEvent(event);
  // 发送失去焦点得信号
  emit sig_onblur();
}


void CustomizeEdit::limit_text_length(QString _text) {
  if (max_len_ <= 0) {
    return;
  }
  QByteArray byteArray = _text.toUtf8();
  if (byteArray.size() > max_len_) {
    byteArray = byteArray.left(max_len_);
    this->setText(QString::fromUtf8(byteArray));
  }
}

void CustomizeEdit::create_connection() {
  connect(this, &QLineEdit::textChanged, this,
          &CustomizeEdit::limit_text_length);
}