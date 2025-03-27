#ifndef REGISTER_DLG_HPP
#define REGISTER_DLG_HPP
#include "../common/constant.hpp"
#include "ui_register_dlg.h"
#include <QDialog>
#include <QString>
#include <QTimer>
#include <functional>
class RegisterDlg : public QDialog {
  Q_OBJECT
public:
  RegisterDlg(QWidget *_parent);

private slots:
  //
  void slot_reg_mod_finished(QString _res, RequestID _req_ID, Modules _modules,
                             ErrorCode _ec);

  void slot_click_confirm_btn();

  void slot_click_get_vertify_btn();

  void slot_click_reg_success_btn();
  
  void slot_click_cancel_btn();
signals:
  void sig_switch_login_page();

private:
  void init_http_handlers();
  void create_connection();
  void show_tip(QString _str, bool _ok);
  void add_tip_err(ErrorCode _ec, QString const &_tip);
  void del_tip_err(ErrorCode _ec);
  bool check_user_valid();
  bool check_confirm_valid();
  bool check_email_valid();
  bool check_pwd_valid();
  bool check_vertify_valid();
  void change_tip_page();

  QMap<ErrorCode, QString> tip_str_map_;
  Ui::RegisterDlg *ui_;
  QMap<RequestID, std::function<void(QJsonObject const &)>> handlers_;
  QTimer *timer_;
  int countdown_;
};

#endif