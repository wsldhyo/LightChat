#include "register_dlg.hpp"
#include "../common/http_manager.hpp"
#include "clickable_lbl.hpp"
#include "global.hpp"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
RegisterDlg::RegisterDlg(QWidget *_parent /*nullptr*/)
    : QDialog(_parent), ui_(new Ui::RegisterDlg()), countdown_(5) {
  ui_->setupUi(this);
  ui_->pwd_edit->setEchoMode(QLineEdit::Password);
  ui_->confirm_pwd_edit->setEchoMode(QLineEdit::Password);
  ui_->err_tip_lbl->setProperty("state", "normal");
  repolish(ui_->err_tip_lbl);
  ui_->err_tip_lbl->clear();
  ui_->pwd_visible_lbl->set_state("unvisible", "unvisible_hover", "", "visible",
                                  "visible_hover", "");
  ui_->confirm_pwd_visible_lbl->set_state("unvisible", "unvisible_hover", "",
                                          "visible", "visible_hover", "");

  timer_ = new QTimer(this);
  init_http_handlers();
  create_connection();
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
          } else if (error == ErrorCode::USER_EXISTED) {
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

    change_tip_page();
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

void RegisterDlg::slot_click_cancel_btn() {
  emit sig_switch_login_page(); // 让MainWindow切换窗口
}

void RegisterDlg::slot_click_reg_success_btn() {
  timer_->stop();               // 停止倒计时
  emit sig_switch_login_page(); // 让MainWindow切换窗口
}

void RegisterDlg::slot_click_confirm_btn() {
  bool valid = check_user_valid();
  if (!valid) {
    qDebug() << "invalid user";
    return;
  }
  valid = check_email_valid();
  if (!valid) {
    qDebug() << "invalid email";
    return;
  }
  valid = check_pwd_valid();
  if (!valid) {
    qDebug() << "invalid pwd";
    return;
  }
  valid = check_confirm_valid();
  if (!valid) {
    qDebug() << "invalid confirm pwd";
    return;
  }
  valid = check_vertify_valid();
  if (!valid) {
    qDebug() << "invalid vertify code";
    return;
  }
  // 发送http请求注册用户
  QJsonObject json_obj;
  json_obj["user"] = ui_->usr_edit->text();
  json_obj["email"] = ui_->email_edit->text();
  QString code_pwd;
  xor_string(ui_->pwd_edit->text(), code_pwd);
  json_obj["passwd"] = code_pwd;
  json_obj["confirm"] = ui_->confirm_pwd_edit->text();
  json_obj["vertifycode"] = ui_->vertify_edit->text();
  HttpManager::get_instance()->post_http_request(
      QUrl(gate_url_prefix + "/usr_register"), json_obj,
      RequestID::REGISTER_USER, Modules::REGISTER_MOD);
}

void RegisterDlg::slot_click_get_vertify_btn() {
  if (check_email_valid()) {
    // 发送http请求获取验证码
    QJsonObject json_obj;
    json_obj["email"] = ui_->email_edit->text();
    HttpManager::get_instance()->post_http_request(
        QUrl(gate_url_prefix + "/get_vertifycode"), json_obj,
        RequestID::GET_VERTIFY_CODE, Modules::REGISTER_MOD);
  }
}

void RegisterDlg::create_connection() {
  connect(ui_->get_vetrify_btn, &QPushButton::clicked, this,
          &RegisterDlg::slot_click_get_vertify_btn);
  connect(ui_->confirm_btn, &QPushButton::clicked, this,
          &RegisterDlg::slot_click_confirm_btn);
  connect(ui_->cancel_btn, &QPushButton::clicked, this,
          &RegisterDlg::slot_click_cancel_btn);
  connect(ui_->reg_success_ret_btn, &QPushButton::clicked, this,
          &RegisterDlg::slot_click_reg_success_btn);
  connect(HttpManager::get_instance().get(), &HttpManager::sig_reg_mod_finished,
          this, &RegisterDlg::slot_reg_mod_finished);
  connect(ui_->usr_edit, &QLineEdit::editingFinished, this,
          [this]() { check_user_valid(); });
  connect(ui_->email_edit, &QLineEdit::editingFinished, this,
          [this]() { check_email_valid(); });
  connect(ui_->pwd_edit, &QLineEdit::editingFinished, this,
          [this]() { check_pwd_valid(); });
  connect(ui_->confirm_pwd_edit, &QLineEdit::editingFinished, this,
          [this]() { check_confirm_valid(); });
  connect(ui_->vertify_edit, &QLineEdit::editingFinished, this,
          [this]() { check_vertify_valid(); });

  // 隐藏和显示密码
  connect(ui_->confirm_pwd_visible_lbl, &ClickableLbl::clicked, [this]() {
    auto state = ui_->confirm_pwd_visible_lbl->get_state();
    if (state == PwdVisibleState::SELECTED) {
      ui_->confirm_pwd_edit->setEchoMode(QLineEdit::Normal);
    } else {
      ui_->confirm_pwd_edit->setEchoMode(QLineEdit::Password);
    }
  });

  // 隐藏和显示密码
  connect(ui_->pwd_visible_lbl, &ClickableLbl::clicked, [this]() {
    auto state = ui_->pwd_visible_lbl->get_state();
    if (state == PwdVisibleState::SELECTED) {
      ui_->pwd_edit->setEchoMode(QLineEdit::Normal);
    } else {
      ui_->pwd_edit->setEchoMode(QLineEdit::Password);
    }
  });

  connect(timer_, &QTimer::timeout, [this]() {
    if (countdown_ == 0) {
      timer_->stop();
      emit sig_switch_login_page(); // 让MainWindow切换窗口
      return;
    }
    countdown_--;
    auto str = QString("注册成功，%1 s后返回登录").arg(countdown_);
    ui_->reg_success_lbl1->setText(str);
  });
}

// 显示错误提示，并将其记录到map中
void RegisterDlg::add_tip_err(ErrorCode _ec, QString const &_tip) {
  tip_str_map_[_ec] = _tip;
  show_tip(_tip, false);
}

// 移除第一个错误提示，显示随后的错误提示
void RegisterDlg::del_tip_err(ErrorCode _ec) {
  tip_str_map_.remove(_ec);
  if (tip_str_map_.empty()) {
    // 没有错误可显示，就清空
    ui_->err_tip_lbl->clear();
    return;
  }
  show_tip(tip_str_map_.first(), false);
}

bool RegisterDlg::check_user_valid() {
  if (ui_->usr_edit->text() == "") {
    add_tip_err(ErrorCode::TIP_USER_ERR, QString("用户名不能为空"));
    return false;
  }
  del_tip_err(ErrorCode::TIP_USER_ERR);
  return true;
}

bool RegisterDlg::check_confirm_valid() {
  auto const &confirm_pwd = ui_->confirm_pwd_edit->text();
  auto const &pwd = ui_->pwd_edit->text();
  add_tip_err(ErrorCode::TIP_CONFIRM_ERR, "两次输入的密码不一致");
  if (confirm_pwd != pwd) {
    return false;
  }
  del_tip_err(ErrorCode::TIP_CONFIRM_ERR);
  return true;
}

bool RegisterDlg::check_email_valid() {
  // 验证邮箱的地址正则表达式
  auto email = ui_->email_edit->text();
  // 邮箱地址的正则表达式
  QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
  bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
  if (!match) {
    if (email.isEmpty()) {
      add_tip_err(ErrorCode::TIP_EMPTY_EMAIL_ERR, "请输入邮箱地址");
    } else {

      add_tip_err(ErrorCode::TIP_EMAIL_ERR, "邮箱地址不正确");
    }
    return false;
  }
  del_tip_err(ErrorCode::TIP_EMAIL_ERR);
  del_tip_err(ErrorCode::TIP_EMPTY_EMAIL_ERR);
  return true;
}

bool RegisterDlg::check_pwd_valid() {
  auto pass = ui_->pwd_edit->text();
  if (pass.length() < 6 || pass.length() > 15) {
    // 提示长度不准确
    add_tip_err(ErrorCode::TIP_PWD_ERR, "密码长度应为6~15");
    return false;
  }
  // 创建一个正则表达式对象，按照上述密码要求
  // 这个正则表达式解释：
  // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
  QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
  bool match = regExp.match(pass).hasMatch();
  if (!match) {
    // 提示字符非法
    add_tip_err(ErrorCode::TIP_PWD_ERR, "不能包含非法字符");
    return false;
    ;
  }
  del_tip_err(ErrorCode::TIP_PWD_ERR);
  return true;
}

bool RegisterDlg::check_vertify_valid() {
  qDebug() << "confirm pwd edit finished";
  auto pass = ui_->vertify_edit->text();
  if (pass.isEmpty()) {
    add_tip_err(ErrorCode::TIP_VARIFY_ERR, "验证码不能为空");
    return false;
  }
  del_tip_err(ErrorCode::TIP_VARIFY_ERR);
  return true;
}

void RegisterDlg::change_tip_page() {
  timer_->stop();
  ui_->stackedWidget->setCurrentWidget(ui_->page_2);
  // 启动定时器，设置间隔为1000毫秒（1秒）
  timer_->start(1000);
}