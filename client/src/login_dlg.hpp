#ifndef LOGIN_DLG_HPP
#define LOGIN_DLG_HPP
#include "ui_logindlg.h"
#include "struct_def.hpp"
#include <QDialog>

class LoginDlg : public QDialog {
  Q_OBJECT
public:
  LoginDlg(QWidget *_parent = nullptr);

private slots:
  void slot_click_login_btn();
  void slot_login_mod_finished(QString _res, RequestID _req_ID,
                               Modules _modules, ErrorCode _ec);
signals:
  void sig_switch_register_dlg();
  void sig_switch_reset_pwd();
  void sig_connect_long_tcp(ServerInfo _si);

private:
  void init_http_handlers();
  void init_head_image();
  void create_connection();
  void enable_btn(bool _enable);
  void add_tip_err(ErrorCode _ec, QString const &_tip);
  void del_tip_err(ErrorCode _ec);
  void show_tip(QString _str, bool _ok);
  bool check_email_valid();
  bool check_pwd_valid();

  QMap<ErrorCode, QString> tip_str_map_;
  QMap<RequestID, std::function<void(QJsonObject const &)>> handlers_;
  Ui::LoginDlg *ui_;
  int uid_;
  QString token_;
};

#endif