#include "reset_pwd_dlg.hpp"
#include "../common/constant.hpp"
#include "../common/http_manager.hpp"
#include "global.hpp"
ResetPwdDlg::ResetPwdDlg(QWidget *_parent /*nullptr*/)
    : QDialog(_parent), ui_(new Ui::ResetPwdDlg) {
  ui_->setupUi(this);
  // 连接reset相关信号和注册处理回调
  init_http_handlers();
  create_connection();
}
void ResetPwdDlg::slot_get_code_btn_clicked() {
  qDebug() << "receive varify btn clicked ";
  auto email = ui_->email_edit->text();
  auto bcheck = check_email_valid();
  if (!bcheck) {
    return;
  }
  // 发送http请求获取验证码
  QJsonObject json_obj;
  json_obj["email"] = email;
  HttpManager::get_instance()->post_http_request(
      QUrl(gate_url_prefix + "/get_vertifycode"), json_obj,
      RequestID::GET_VERTIFY_CODE, Modules::RESET_PWD_MOD);
}
void ResetPwdDlg::slot_return_btn_clicked() { emit sig_switch_login_page(); }
void ResetPwdDlg::slot_confirme_btn_clicked() {
  qDebug() << "confirm btn press...check args";
  bool valid = check_user_valid();
  if (!valid) {
    return;
  }
  valid = check_email_valid();
  if (!valid) {
    return;
  }
  valid = check_pwd_valid();
  if (!valid) {
    return;
  }
  valid = check_vertify_valid();
  if (!valid) {
    return;
  }
  // 发送http重置用户请求
  QJsonObject json_obj;
  json_obj["user"] = ui_->user_edit->text();
  json_obj["email"] = ui_->email_edit->text();
  QString res;
  xor_string(ui_->new_pwd_edit->text(), res);
  json_obj["passwd"] = res;
  json_obj["vertifycode"] = ui_->vertify_code_edit->text();
  HttpManager::get_instance()->post_http_request(
      QUrl(gate_url_prefix + "/reset_pwd"), json_obj, RequestID::RESET_PWD,
      Modules::RESET_PWD_MOD);
}
void ResetPwdDlg::slot_reset_mod_finished(QString _res, RequestID _req_ID,
                                          Modules _modules, ErrorCode _err) {
  if (_err != ErrorCode::NO_ERROR) {
    show_tip(tr("网络请求错误"), false);
    return;
  }
  // 解析 JSON 字符串,res需转化为QByteArray
  QJsonDocument jsonDoc = QJsonDocument::fromJson(_res.toUtf8());
  // json解析错误
  if (jsonDoc.isNull()) {
    show_tip(tr("json解析错误"), false);
    return;
  }
  // json解析错误
  if (!jsonDoc.isObject()) {
    show_tip(tr("json解析错误"), false);
    return;
  }
  // 调用对应的逻辑,根据id回调。
  handlers_[_req_ID](jsonDoc.object());
  return;
}

void ResetPwdDlg::init_http_handlers() {
  // 注册获取验证码回包逻辑
  handlers_.insert(RequestID::GET_VERTIFY_CODE, [this](QJsonObject jsonObj) {
    int error = jsonObj["error"].toInt();
    if (error != static_cast<int>(ErrorCode::NO_ERROR)) {
      show_tip(tr("参数错误"), false);
      return;
    }
    auto email = jsonObj["email"].toString();
    show_tip(tr("验证码已发送到邮箱，注意查收"), true);
    qDebug() << "email is " << email;
  });
  // 注册重置密码的回包逻辑
  handlers_.insert(RequestID::RESET_PWD, [this](QJsonObject jsonObj) {
    int error = jsonObj["error"].toInt();

    auto err = static_cast<ErrorCode>(error);
    switch (err) {
    case ErrorCode::VERTIFY_CODE_DISMATCH:
      show_tip(tr("验证码不正确"), false);
      break;
    case ErrorCode::VERTIFY_CODE_EXPIRED:
      show_tip(tr("验证码已过期"), false);
      break;
    case ErrorCode::EMAIL_DISMATCH:
      show_tip(tr("用户名与邮箱不匹配"), false);
      break;
    case ErrorCode::NO_ERROR: {
      auto email = jsonObj["email"].toString();
      show_tip(tr("重置成功,点击返回登录"), true);
      qDebug() << "email is " << email;
      qDebug() << "user uui_d is " << jsonObj["uui_d"].toString();
    } break;
    default:
      qDebug() << "reset pwd error is " << error;
      show_tip("网络错误", false);
      break;
    }
  });
}
void ResetPwdDlg::create_connection() {
  connect(ui_->return_btn, &QPushButton::clicked, this,
          &ResetPwdDlg::slot_return_btn_clicked);
  connect(ui_->confirm_btn, &QPushButton::clicked, this,
          &ResetPwdDlg::slot_confirme_btn_clicked);
  connect(ui_->get_code_btn, &QPushButton::clicked, this,
          &ResetPwdDlg::slot_get_code_btn_clicked);
  connect(ui_->user_edit, &QLineEdit::editingFinished, this,
          [this]() { check_user_valid(); });
  connect(ui_->email_edit, &QLineEdit::editingFinished, this,
          [this]() { check_email_valid(); });
  connect(ui_->new_pwd_edit, &QLineEdit::editingFinished, this,
          [this]() { check_pwd_valid(); });
  connect(ui_->vertify_code_edit, &QLineEdit::editingFinished, this,
          [this]() { check_vertify_valid(); });
  connect(HttpManager::get_instance().get(),
          &HttpManager::sig_reset_pwd_mod_finished, this,
          &ResetPwdDlg::slot_reset_mod_finished);
}

void ResetPwdDlg::show_tip(QString _str, bool _ok) {
  if (_ok) {
    ui_->err_tip_lbl->setProperty("state", "normal");
  } else {
    ui_->err_tip_lbl->setProperty("state", "err");
  }
  ui_->err_tip_lbl->setText(_str);
  repolish(ui_->err_tip_lbl);
}
void ResetPwdDlg::add_tip_err(ErrorCode _ec, QString const &_tip) {
  tip_str_map_[_ec] = _tip;
  show_tip(_tip, false);
}
void ResetPwdDlg::del_tip_err(ErrorCode _ec) {
  tip_str_map_.remove(_ec);
  if (tip_str_map_.empty()) {
    ui_->err_tip_lbl->clear();
    return;
  }
  show_tip(tip_str_map_.first(), false);
}
bool ResetPwdDlg::check_user_valid() {
  if (ui_->user_edit->text() == "") {
    add_tip_err(ErrorCode::TIP_USER_ERR, tr("用户名不能为空"));
    return false;
  }
  del_tip_err(ErrorCode::TIP_USER_ERR);
  return true;
}

bool ResetPwdDlg::check_email_valid() {
  // 验证邮箱的地址正则表达式
  auto email = ui_->email_edit->text();
  // 邮箱地址的正则表达式
  QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
  bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
  if (!match) {
    // 提示邮箱不正确
    add_tip_err(ErrorCode::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
    return false;
  }
  del_tip_err(ErrorCode::TIP_EMAIL_ERR);
  return true;
}
bool ResetPwdDlg::check_pwd_valid() {
  auto pass = ui_->new_pwd_edit->text();
  if (pass.length() < 6 || pass.length() > 15) {
    // 提示长度不准确
    add_tip_err(ErrorCode::TIP_PWD_ERR, tr("密码长度应为6~15"));
    return false;
  }
  // 创建一个正则表达式对象，按照上述密码要求
  // 这个正则表达式解释：
  // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
  QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
  bool match = regExp.match(pass).hasMatch();
  if (!match) {
    // 提示字符非法
    add_tip_err(ErrorCode::TIP_PWD_ERR, tr("不能包含非法字符"));
    return false;
    ;
  }
  del_tip_err(ErrorCode::TIP_PWD_ERR);
  return true;
}
bool ResetPwdDlg::check_vertify_valid() {
  auto pass = ui_->vertify_code_edit->text();
  if (pass.isEmpty()) {
    add_tip_err(ErrorCode::TIP_VARIFY_ERR, tr("验证码不能为空"));
    return false;
  }
  del_tip_err(ErrorCode::TIP_VARIFY_ERR);
  return true;
}
void ResetPwdDlg::change_tip_page() {}