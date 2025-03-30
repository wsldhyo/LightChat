#include "login_dlg.hpp"
#include "../common/http_manager.hpp"
#include "global.hpp"
#include "tcp_manager.hpp"
#include <QPainter>
#include <QPainterPath>
LoginDlg::LoginDlg(QWidget *_parent /*nullptr*/) : ui_(new Ui::LoginDlg) {
  ui_->setupUi(this);
  ui_->pwd_edit->setEchoMode(QLineEdit::Password);
  ui_->forget_pwd_lbl->set_state("normal", "hover", "", "selected",
                                 "selected_hover", "");
  init_head_image();
  init_http_handlers();
  create_connection();
}

void LoginDlg::init_http_handlers() {

  // 注册获取登录回包逻辑
  handlers_.insert(RequestID::LOGIN_USER, [this](QJsonObject jsonObj) {
    // 收到服务器回包就就取消对登录和注册按钮的禁用
    int error = jsonObj["error"].toInt();
    if (error != static_cast<int>(ErrorCode::NO_ERROR)) {
      switch (static_cast<ErrorCode>(error)) {
      case ErrorCode::PWD_DISMATCH:
        show_tip("用户名与密码不匹配", false);
        break;
      default:
        show_tip("网络参数错误", false);
        break;
      }
      // 登录失败，使能相关按钮
      enable_btn(true);
      return;
    }

    auto user = jsonObj["user"].toString();
    show_tip("", true);

    ServerInfo server_info;
    server_info.host = jsonObj["host"].toString();
    server_info.port = jsonObj["port"].toString();
    server_info.token = jsonObj["token"].toString();
    server_info.uid = jsonObj["uid"].toInt();

    qDebug() << "token: " << server_info.token << " uid:" << server_info.uid;
    // 建立TCP长连接的信号, TCP长连接为聊天会话服务
    emit sig_connect_tcp(server_info);
  });
}
void LoginDlg::slot_click_login_btn() {
  qDebug() << "login btn clicked";
  if (!check_email_valid()) {
    return;
  }
  if (!check_pwd_valid()) {
    return;
  }
  enable_btn(false);
  auto user = ui_->email_edit->text();
  auto pwd = ui_->pwd_edit->text();
  // 发送http请求登录
  QJsonObject json_obj;
  json_obj["user"] = user;
  QString res;
  xor_string(pwd, res);
  json_obj["passwd"] = res;
  HttpManager::get_instance()->post_http_request(
      QUrl(gate_url_prefix + "/user_login"), json_obj, RequestID::LOGIN_USER,
      Modules::USER_LOGIN_MOD);
}

void LoginDlg::slot_login_mod_finished(QString _res, RequestID _req_ID,
                                       Modules _modules, ErrorCode _ec) {
  if (_ec != ErrorCode::NO_ERROR) {
    show_tip("网络请求错误", false);
    enable_btn(true);
    return;
  }
  // 解析 JSON 字符串,res需转化为QByteArray
  QJsonDocument jsonDoc = QJsonDocument::fromJson(_res.toUtf8());
  // json解析错误
  if (jsonDoc.isNull()) {
    show_tip("json解析错误", false);
    enable_btn(true);
    return;
  }
  // json解析错误
  if (!jsonDoc.isObject()) {
    enable_btn(true);
    show_tip("json解析错误", false);
    return;
  }
  // 调用对应的逻辑,根据id回调。
  handlers_[_req_ID](jsonDoc.object());
  return;
}

void LoginDlg::slot_tcp_connect_finished(bool _success) {

  if (_success) {
    show_tip("聊天服务连接成功，正在登录...", true);
    QJsonObject jsonObj;
    jsonObj["uid"] = uid_;
    jsonObj["token"] = token_;
    QJsonDocument doc(jsonObj);
    QString jsonString = doc.toJson(QJsonDocument::Indented);
    // 发送tcp请求给chat server，请求登录聊天服务器
    TcpManager::get_instance()->sig_send_data(RequestID::LOGIN_CHAT_SERVER,
                                              jsonString);
  } else {
    show_tip("网络异常", false);
    enable_btn(true);
  }
}

void LoginDlg::init_head_image() {
  // 加载默认头像图片
  QPixmap originalPixmap(":/icons/head_1.jpg");
  qDebug() << originalPixmap.size() << ui_->head_lbl->size();
  // 设置图片自动缩放
  originalPixmap = originalPixmap.scaled(
      ui_->head_lbl->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);

  // 创建一个和原始图片相同大小的QPixmap，
  QPixmap roundedPixmap(originalPixmap.size());
  roundedPixmap.fill(Qt::transparent); // 用透明色填充，用于绘制圆角图片

  // 准备绘制圆角图片
  QPainter painter(&roundedPixmap);
  painter.setRenderHint(QPainter::Antialiasing); // 设置抗锯齿，使圆角更平滑
  painter.setRenderHint(QPainter::SmoothPixmapTransform);

  // 使用QPainterPath设置圆角
  QPainterPath path;
  path.addRoundedRect(0, 0, originalPixmap.width(), originalPixmap.height(), 10,
                      10); // 最后两个参数分别是x和y方向的圆角半径
  painter.setClipPath(path);

  // 将原始图片绘制到roundedPixmap上
  painter.drawPixmap(0, 0, originalPixmap);

  // 设置绘制好的圆角图片到QLabel上
  ui_->head_lbl->setPixmap(roundedPixmap);
}

void LoginDlg::create_connection() {
  connect(ui_->signup_btn, &QPushButton::clicked, this,
          &LoginDlg::sig_switch_register_dlg);
  connect(ui_->forget_pwd_lbl, &ClickableLbl::clicked, this,
          &LoginDlg::sig_switch_reset_pwd);
  connect(ui_->login_btn, &QPushButton::clicked, this,
          &LoginDlg::slot_click_login_btn);

  connect(HttpManager::get_instance().get(),
          &HttpManager::sig_login_mod_finished, this,
          &LoginDlg::slot_login_mod_finished);

  connect(this, &LoginDlg::sig_connect_tcp, TcpManager::get_instance().get(),
          &TcpManager::slot_tcp_connect);
}

// 点击登录后，禁用注册按钮和登录按钮
void LoginDlg::enable_btn(bool _enable) {
  ui_->login_btn->setEnabled(_enable);
  ui_->signup_btn->setEnabled(_enable);
}

void LoginDlg::add_tip_err(ErrorCode _ec, QString const &_tip) {
  tip_str_map_[_ec] = _tip;
  show_tip(_tip, false);
}

void LoginDlg::del_tip_err(ErrorCode _ec) {

  tip_str_map_.remove(_ec);
  if (tip_str_map_.empty()) {
    // 没有错误可显示，就清空
    ui_->tip_lbl->clear();
    return;
  }
  show_tip(tip_str_map_.first(), false);
}
bool LoginDlg::check_email_valid() {

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

bool LoginDlg::check_pwd_valid() {

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

void LoginDlg::show_tip(QString _str, bool _ok) {

  if (!_ok) {
    ui_->tip_lbl->setProperty("state", "err");
  } else {

    ui_->tip_lbl->setProperty("state", "normal");
  }
  ui_->tip_lbl->setText(_str);
  repolish(ui_->tip_lbl);
}