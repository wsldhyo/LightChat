#include "register_dlg.hpp"
#include "../common/http_manager.hpp"
#include "global.hpp"
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <qdebug.h>
#include <qjsondocument.h>
#include <qjsonobject.h>
RegisterDlg::RegisterDlg(QWidget *_parent /*nullptr*/)
    : QDialog(_parent), ui_(new Ui::RegisterDlg()) {
  ui_->setupUi(this);
  ui_->pwd_edit->setEchoMode(QLineEdit::Password);
  ui_->confirm_pwd_edit->setEchoMode(QLineEdit::Password);
  ui_->err_tip_lbl->setProperty("state", "normal");
  repolish(ui_->err_tip_lbl);
  init_http_handlers();
  create_connection();
}

void RegisterDlg::slot_get_code_clicked() {

  auto email = ui_->email_edit->text();
  QRegularExpression emailRegex(
      R"((^[a-zA-Z0-9_.+-]+@[a-zA-Z0-9-]+\.[a-zA-Z0-9-.]+$))");
  bool match = emailRegex.match(email).hasMatch();
  if (match) {
    QJsonObject json_obj;
    json_obj["email"] = email;
    HttpManager::get_instance()->post_http_request(
        QUrl(gate_url_prefix + "/get_vertifycode"), json_obj,
        RequestID::GET_VERTIFY_CODE, Modules::REGISTER_MOD);
    
  } else {
    show_tip("邮箱地址不正确", false);
  }
}

void RegisterDlg::slot_reg_mod_finished(QString _res, RequestID _req_ID,
                                        Modules _modules, ErrorCode _ec) {
  if (_ec != ErrorCode::NO_ERROR) {
    show_tip("网络请求错误", false);
    return;
  }
  // 解析JSON， res先转为utf-8编码格式的字节流
  QJsonDocument json_doc = QJsonDocument::fromJson(_res.toUtf8());
  if (json_doc.isNull()) {
    show_tip("JSON 解析失败", false);
    return;
  }

  if (!json_doc.isObject()) {
    show_tip("JSON 解析失败", false);
    return;
  }
  auto handler_it = handlers_.find(_req_ID);
  if (handler_it == handlers_.end()) {
    show_tip("无法处理该类型请求", false);
    return;
  }
  show_tip("验证码请求已发出", true);
  handler_it.value()(json_doc.object());
  return;
}

void RegisterDlg::init_http_handlers() {
  // 注册各种类型请求的回调函数
  handlers_.insert(
      RequestID::GET_VERTIFY_CODE, [this](QJsonObject const &_json_obj) {
        auto error = static_cast<ErrorCode>(_json_obj["error"].toInt());
        if (error != ErrorCode::NO_ERROR) {
          show_tip("参数错误", false);
          return;
        }
        auto email = _json_obj["email"].toString();
        show_tip("验证码已经发送到指定邮箱，请注意查收", true);
        qDebug() << "email is " << email;
      });
}

void RegisterDlg::show_tip(QString _str, bool _ok) {
  if (!_ok) {
    ui_->err_tip_lbl->setProperty("state", "err");
  } else {

    ui_->err_tip_lbl->setProperty("state", "normal");
  }
  ui_->err_tip_lbl->setText(_str);
  repolish(ui_->err_tip_lbl);
}

void RegisterDlg::create_connection() {
  connect(ui_->get_vetrify_btn, &QPushButton::clicked, this,
          &RegisterDlg::slot_get_code_clicked);
  connect(HttpManager::get_instance().get(), &HttpManager::sig_reg_mod_finished,
          this, &RegisterDlg::slot_reg_mod_finished);
}