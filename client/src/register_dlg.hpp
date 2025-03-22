#ifndef REGISTER_DLG_HPP
#define REGISTER_DLG_HPP
#include "../common/constant.hpp"
#include "ui_register_dlg.h"
#include <QDialog>
#include <QString>
#include <functional>
class RegisterDlg : public QDialog {
  Q_OBJECT
public:
  RegisterDlg(QWidget *_parent);

private slots:
  // 处理获取验证码按钮点击信号
  void slot_get_code_clicked();
  //
  void slot_reg_mod_finished(QString _res, RequestID _req_ID, Modules _modules,
                             ErrorCode _ec);

  void slot_confirm_register_user();
private:
  void init_http_handlers();
  void create_connection();
  void show_tip(QString _str, bool _ok);
  Ui::RegisterDlg *ui_;
  QMap<RequestID, std::function<void(QJsonObject const&)>> handlers_;
};

#endif