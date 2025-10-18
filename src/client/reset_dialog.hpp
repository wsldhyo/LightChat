#ifndef RESET_DIALOG_HPP
#define RESET_DIALOG_HPP

#include "client_constant.hpp"
#include "utility/constant.hpp"
#include <QDialog>
#include <QMap>

namespace Ui {
class ResetDialog;
}

class ResetDialog : public QDialog {
  Q_OBJECT

public:
  explicit ResetDialog(QWidget *parent = nullptr);
  ~ResetDialog();

private slots:
  void on_return_btn_clicked();

  void on_varify_btn_clicked();

  void slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
  void on_sure_btn_clicked();

signals:
  void sig_switch_login();
private:
  bool check_user_valid();
  bool check_pass_valid();
  void show_tip(QString str, bool b_ok);
  bool check_email_valid();
  bool check_veritfy_valid();
  void add_tip_err(TipErr te, QString tips);
  void del_tip_err(TipErr te);
  void init_handlers();
  Ui::ResetDialog *ui;
  QMap<TipErr, QString> tip_errs_;
  QMap<ReqId, std::function<void(const QJsonObject &)>> handlers_;
};
#endif