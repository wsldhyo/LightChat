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

private:
  bool checkUserValid();
  bool checkPassValid();
  void showTip(QString str, bool b_ok);
  bool checkEmailValid();
  bool checkVarifyValid();
  void AddTipErr(TipErr te, QString tips);
  void DelTipErr(TipErr te);
  void initHandlers();
  Ui::ResetDialog *ui;
  QMap<TipErr, QString> tip_errs_;
  QMap<ReqId, std::function<void(const QJsonObject &)>> handlers_;
signals:
  void switchLogin();
};
#endif