#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QMap>
#include <functional>

#include "client_constant.hpp"
#include "utility/constant.hpp"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog {
  Q_OBJECT

public:
  explicit RegisterDialog(QWidget *parent = nullptr);
  ~RegisterDialog();

private slots:
  void on_get_code_clicked();
  void on_sure_btn_clicked();

  void on_return_btn_clicked();

  void on_cancel_btn_clicked();

public slots:
  // 处理注册请求完成
  void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);

private:
  bool check_user_valid();
  bool check_email_valid();
  bool check_pass_valid();
  bool check_vertify_valid();
  bool check_confirm_valid();

  // 初始化回调函数集
  void init_http_handlers();

  /**
    @brief 设置err_tip的提示信息和样式
    @param str 提示信息
    @param b_ok
          true: 正确，样式为绿色
          false: 错误，样式为红色
    @return void
  */
  void show_tip(QString str, bool b_ok);
  void create_connection();

  void add_tip_err(TipErr te, QString tips);
  void del_tip_err(TipErr te);
  void change_tip_page();

signals:
  void sig_switch_login();

private:
  Ui::RegisterDialog *ui;
  // 处理Http请求完成后的回调函数集
  QMap<ReqId, std::function<void(const QJsonObject &)>> handlers_;
  QMap<TipErr, QString> tip_errs_;
  QTimer *countdown_timer_;
  int countdown_;
};

#endif // REGISTERDIALOG_H
