#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include "client_constant.hpp"
#include "client_struct_def.hpp"
#include "utility/constant.hpp"
#include <QDialog>
#include <QJsonObject>
#include <QMap>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog {
  Q_OBJECT

public:
  explicit LoginDialog(QWidget *parent = nullptr);
  ~LoginDialog();
private slots:
  void slot_forget_pwd();
  void on_login_btn_clicked();
  void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);
  void slot_tcp_con_finish(bool bsuccess);
  void slot_login_failed(int);
signals:
  void switchRegister(); // 跳转到注册界面的信号
  void switchReset();
  void sig_connect_tcp(ServerInfo);

private:
  void initHttpHandlers();
  // 为登录界面的头像框绘制图片
  void initHeadLabel();
  bool enableBtn(bool);
  void showTip(QString str, bool b_ok);
  bool checkUserValid();
  bool checkPwdValid();
  void AddTipErr(TipErr te, QString tips);
  void DelTipErr(TipErr te);
  void create_connection();

private:
  Ui::LoginDialog *ui;
  QMap<TipErr, QString> tip_errs_;
  QMap<ReqId, std::function<void(const QJsonObject &)>> handlers_;
  int uid_;
  QString token_;
};

#endif // LOGINDIALOG_H
