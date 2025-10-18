#include "customize_edit.hpp"

CustomizeEdit::CustomizeEdit(QWidget *parent) : QLineEdit(parent), max_len_(0) {
  connect(this, &QLineEdit::textChanged, this, &CustomizeEdit::limit_text_length);
}

void CustomizeEdit::set_max_length(int maxLen) { max_len_ = maxLen; }

void CustomizeEdit::focusOutEvent(QFocusEvent *event) {
  // 执行失去焦点时的处理逻辑
  // qDebug() << "CustomizeEdit focusout";
  // 调用基类的focusOutEvent()方法，保证基类的行为得到执行
  QLineEdit::focusOutEvent(event);
  //发送失去焦点得信号
  emit sig_foucus_out();
}

void CustomizeEdit::limit_text_length(QString text) {
  if (max_len_ <= 0) {
    return;
  }

  QByteArray byteArray = text.toUtf8();
  // 输入的可能是多字节数据如中文，所以按照字节数截取
  if (byteArray.size() > max_len_) {
    byteArray = byteArray.left(max_len_);
    this->setText(QString::fromUtf8(byteArray));
  }
}