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
  qDebug() << "handle http response";
  if (_req_ID == RequestID::GET_VERTIFY_CODE) {
    show_tip("验证码请求已发出", true);
  } else if (_req_ID == RequestID::REGISTER_USER) {
    show_tip("注册用户请求已发出，正在查询注册结果中", true);
  }
  handler_it.value()(json_doc.object());
  return;
}

void RegisterDlg::init_http_handlers() {
  // 注册各种类型请求的回调函数
  // 验证码请求处理
  handlers_.insert(
      RequestID::GET_VERTIFY_CODE, [this](QJsonObject const &_json_obj) {
        auto error = static_cast<ErrorCode>(_json_obj["error"].toInt());
        if (error != ErrorCode::NO_ERROR) {
          if (error == ErrorCode::VERTIFY_CODE_EXPIRED) {
            show_tip("验证码已过期，请重新获取", false);
          }
          else if(error == ErrorCode::USER_EXISTED){
            show_tip("用户已存在", false);
          }
          show_tip("参数错误", false);
          return;
        }
        auto email = _json_obj["email"].toString();
        show_tip("验证码已经发送到指定邮箱，请注意查收", true);
        qDebug() << "email is " << email;
      });

  // 注册用户请求
  handlers_.insert(RequestID::REGISTER_USER, [this](QJsonObject jsonObj) {
    auto error = static_cast<ErrorCode>(jsonObj["error"].toInt());
    if (error != ErrorCode::NO_ERROR) {
      show_tip(tr("参数错误"), false);
      return;
    }
    auto email = jsonObj["email"].toString();
    show_tip(tr("用户注册成功"), true);
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

void RegisterDlg::slot_confirm_register_user() {
  if (ui_->usr_edit->text() == "") {
    show_tip(tr("用户名不能为空"), false);
    return;
  }
  if (ui_->email_edit->text() == "") {
    show_tip(tr("邮箱不能为空"), false);
    return;
  }
  if (ui_->pwd_edit->text() == "") {
    show_tip(tr("密码不能为空"), false);
    return;
  }
  if (ui_->confirm_pwd_edit->text() == "") {
    show_tip(tr("确认密码不能为空"), false);
    return;
  }
  if (ui_->confirm_pwd_edit->text() != ui_->pwd_edit->text()) {
    show_tip(tr("密码和确认密码不匹配"), false);
    return;
  }
  if (ui_->vertify_edit->text() == "") {
    show_tip(tr("验证码不能为空"), false);
    return;
  }
  // day11 发送http请求注册用户
  QJsonObject json_obj;
  json_obj["user"] = ui_->usr_edit->text();
  json_obj["email"] = ui_->email_edit->text();
  json_obj["passwd"] = ui_->pwd_edit->text();
  json_obj["confirm"] = ui_->confirm_pwd_edit->text();
  json_obj["vertifycode"] = ui_->vertify_edit->text();
  HttpManager::get_instance()->post_http_request(
      QUrl(gate_url_prefix + "/usr_register"), json_obj,
      RequestID::REGISTER_USER, Modules::REGISTER_MOD);
}

void RegisterDlg::create_connection() {
  connect(ui_->get_vetrify_btn, &QPushButton::clicked, this,
          &RegisterDlg::slot_get_code_clicked);
  connect(ui_->confirm_btn, &QPushButton::clicked, this,
          &RegisterDlg::slot_confirm_register_user);
  connect(HttpManager::get_instance().get(), &HttpManager::sig_reg_mod_finished,
          this, &RegisterDlg::slot_reg_mod_finished);
}