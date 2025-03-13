#include "register_dlg.hpp"
#include "global.hpp"
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <qpushbutton.h>
#include <qvalidator.h>
RegisterDlg::RegisterDlg(QWidget *_parent /*nullptr*/)
    : QDialog(_parent), ui_(new Ui::RegisterDlg()) {
  ui_->setupUi(this);
  ui_->pwd_edit->setEchoMode(QLineEdit::Password);
  ui_->confirm_pwd_edit->setEchoMode(QLineEdit::Password);
  ui_->err_tip_lbl->setProperty("state", "normal");
  repolish(ui_->err_tip_lbl);
  create_connection();
}

void RegisterDlg::slot_get_code_clicked() {

  QRegularExpression emailRegex(
      R"((^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$))");
  bool match = emailRegex.match(ui_->email_edit->text()).hasMatch();
  if (match) {
    // TODO 发送http验证码
  } else {
    show_tip("邮箱地址不正确", false);
  }
}

void RegisterDlg::show_tip(QString _str, bool _ok) {
  if (!_ok) {
    ui_->err_tip_lbl->setProperty("state", "err");
  } else {

    ui_->err_tip_lbl->setProperty("state", "normal");
  }
  ui_->err_tip_lbl->setText(_str);
  repolish(ui_->err_tip_lbl);
}

void RegisterDlg::create_connection() {
  connect(ui_->get_vetrify_btn, &QPushButton::clicked, this,
          &RegisterDlg::slot_get_code_clicked);
}