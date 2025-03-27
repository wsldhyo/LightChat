#ifndef RESET_PWD_DLG_HPP
#define RESET_PWD_DLG_HPP
#include "../common/constant.hpp"
#include "ui_reset_pwd_dlg.h"
#include <QDialog>
#include <QJsonObject>
class ResetPwdDlg : public QDialog {
  Q_OBJECT
public:
  ResetPwdDlg(QWidget *_parent = nullptr);

private slots:
  void slot_get_code_btn_clicked();
  void slot_return_btn_clicked();
  void slot_confirme_btn_clicked();
  void slot_reset_mod_finished(QString _res, RequestID _req_ID,
                               Modules _modules, ErrorCode _err);
signals:
  void sig_switch_login_page();

private:
  void init_http_handlers();
  void create_connection();
  void show_tip(QString _str, bool _ok);
  void add_tip_err(ErrorCode _ec, QString const &_tip);
  void del_tip_err(ErrorCode _ec);
  bool check_user_valid();
  bool check_email_valid();
  bool check_pwd_valid();
  bool check_vertify_valid();
  void change_tip_page();
  Ui::ResetPwdDlg *ui_;
  QMap<ErrorCode, QString> tip_str_map_;
  QMap<RequestID, std::function<void(QJsonObject const &)>> handlers_;
};

#endif