#ifndef LOGIN_DLG_HPP
#define LOGIN_DLG_HPP
#include "ui_logindlg.h"
#include <QDialog>

class LoginDlg : public QDialog {
  Q_OBJECT
public:
  LoginDlg(QWidget *_parent = nullptr);
signals:
  void sig_switch_register_dlg();
  void sig_switch_reset_pwd(); 

private:
  void create_connection();

  Ui::LoginDlg *ui_;
};

#endif