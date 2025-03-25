#include "login_dlg.hpp"
#include <QDebug>
LoginDlg::LoginDlg(QWidget *_parent /*nullptr*/) : ui_(new Ui::LoginDlg) {
  ui_->setupUi(this);
  ui_->pwd_edit->setEchoMode(QLineEdit::Password);
  ui_->forget_pwd_lbl->set_state("normal", "hover", "", "selected",
                                 "selected_hover", "");
  create_connection();
}

void LoginDlg::create_connection() {
  connect(ui_->signup_btn, &QPushButton::clicked, this,
          &LoginDlg::sig_switch_register_dlg);
  connect(ui_->forget_pwd_lbl, &ClickableLbl::clicked, this,
          &LoginDlg::sig_switch_reset_pwd);
}

