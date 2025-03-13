#include "login_dlg.hpp"
LoginDlg::LoginDlg(QWidget* _parent/*nullptr*/):ui_(new Ui::LoginDlg)
{
    ui_->setupUi(this);    
    ui_->pwd_edit->setEchoMode(QLineEdit::Password);
    create_connection();
}

void LoginDlg::create_connection()
{
    connect(ui_->signup_btn, &QPushButton::clicked, this,& LoginDlg::switch_register_dlg);
}