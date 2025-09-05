#include "register_dialog.hpp"
#include "clicked_label.hpp"
#include "client_globalvar.hpp"
#include "http_manager.hpp"
#include "ui_registerdialog.h"
#include <QDebug>
#include <QRegularExpression>
#include <QTimer>
RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::RegisterDialog) {
  ui->setupUi(this);

  //设置窗口无边框
  setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  //设置密码格式隐藏
  ui->pass_edit->setEchoMode(QLineEdit::Password);
  ui->confirm_edit->setEchoMode(QLineEdit::Password);
  //  设置错误提示的初始样式
  ui->err_tip->setProperty("state", "normal");
  g_repolish(ui->err_tip);
  create_connection();
  initHttpHandlers();
  //设置浮动显示手形状
  ui->pass_visible->setCursor(Qt::PointingHandCursor);
  ui->confirm_visible->setCursor(Qt::PointingHandCursor);

  ui->pass_visible->SetState("unvisible", "unvisible_hover", "", "visible",
                             "visible_hover", "");

  ui->confirm_visible->SetState("unvisible", "unvisible_hover", "", "visible",
                                "visible_hover", "");
}

RegisterDialog::~RegisterDialog() {
  qDebug() << "destruct RegDlg";
  delete ui;
}

void RegisterDialog::on_get_code_clicked() {
  //验证邮箱的地址正则表达式
  auto email = ui->email_edit->text();
  // 邮箱地址的正则表达式
  qDebug() << "email: " << email;
  QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
  bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
  qDebug() << "match: " << match;
  if (match) {
    //发送http请求获取验证码
    QJsonObject json_obj;
    json_obj["email"] = email;
    QString url(g_gate_url_prefix);
    url += QString::fromLatin1(POST_GET_VERFIY_CODE.data(),
                               static_cast<int>(POST_GET_VERFIY_CODE.size()));
    HttpMgr::getinstance()->post_http_req(
        QUrl(url), json_obj, ReqId::ID_GET_VERTIFY_CODE, Modules::REGISTERMOD);

  } else {
    //提示邮箱不正确

    showTip(QString("邮箱地址不正确"), false);
  }
}
void RegisterDialog::on_sure_btn_clicked() {

  // 检查输入是否都正确
  bool valid = checkUserValid();
  if (!valid) {
    return;
  }

  valid = checkEmailValid();
  if (!valid) {
    return;
  }

  valid = checkPassValid();
  if (!valid) {
    return;
  }

  valid = checkVarifyValid();
  if (!valid) {
    return;
  }

  // 输入无误后发送http请求注册用户
  QJsonObject json_obj;
  json_obj["user"] = ui->user_edit->text();
  json_obj["email"] = ui->email_edit->text();
  json_obj["pwd"] = xor_string(ui->pass_edit->text());
  json_obj["confirm"] = xor_string(ui->confirm_edit->text());
  json_obj["vertifycode"] = ui->varify_edit->text();
  HttpMgr::getinstance()->post_http_req(
      QUrl(g_gate_url_prefix +
           QString::fromLatin1(POST_REG_USER.data(),
                               static_cast<int>(POST_REG_USER.size()))),
      json_obj, ReqId::ID_REG_USER, Modules::REGISTERMOD);
}

void RegisterDialog::on_return_btn_clicked() {
  countdown_timer_->stop();
  emit sigSwitchLogin();
}

void RegisterDialog::on_cancel_btn_clicked() {
  countdown_timer_->stop();
  emit sigSwitchLogin();
}

void RegisterDialog::initHttpHandlers() {
  //注册获取验证码回包逻辑
  handlers_.insert(ReqId::ID_GET_VERTIFY_CODE, [this](QJsonObject jsonObj) {
    int error = jsonObj["error"].toInt();
    if (error != static_cast<int32_t>(ErrorCodes::NO_ERROR)) {
      showTip("参数错误", false);
      return;
    }
    auto email = jsonObj["email"].toString();
    showTip("验证码已发送到邮箱，注意查收", true);
    qDebug() << "success get vertify code, email is " << email;
  });

  //注册注册用户回包逻辑
  handlers_.insert(ReqId::ID_REG_USER, [this](QJsonObject jsonObj) {
    int error = jsonObj["error"].toInt();
    if (error != static_cast<int32_t>(ErrorCodes::NO_ERROR)) {
      showTip(tr("参数错误"), false);
      return;
    }
    auto email = jsonObj["email"].toString();
    showTip(tr("用户注册成功"), true);
    qDebug() << "email is " << email;
    qDebug() << "user uuid is " << jsonObj["uuid"].toString();
    ChangeTipPage();
  });
}

void RegisterDialog::showTip(QString str, bool b_ok) {
  if (b_ok) {
    ui->err_tip->setProperty("state", "normal");
  } else {
    ui->err_tip->setProperty("state", "err");
  }
  ui->err_tip->setText(str);
  g_repolish(ui->err_tip);
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res,
                                         ErrorCodes err) {
  if (err != ErrorCodes::NO_ERROR) {
    showTip("网络请求错误", false);
    return;
  }
  // 解析 JSON 字符串,res需转化为QByteArray
  QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
  // json解析错误
  if (jsonDoc.isNull()) {
    showTip("json解析错误", false);
    return;
  }
  // json解析错误
  if (!jsonDoc.isObject()) {
    showTip("json解析错误", false);
    return;
  }
  QJsonObject jsonObj = jsonDoc.object();
  //调用对应的处理逻辑,根据id回调。
  auto handler = handlers_.find(id);
  if (handler == handlers_.end()) {
    qDebug() << "register reply handler not found, id: "
             << static_cast<int32_t>(id);
    return;
  }
  handler.value()(jsonDoc.object());
  return;
}

bool RegisterDialog::checkUserValid() {
  if (ui->user_edit->text() == "") {
    AddTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
    return false;
  }

  DelTipErr(TipErr::TIP_USER_ERR);
  return true;
}

bool RegisterDialog::checkPassValid() {
  auto pass = ui->pass_edit->text();

  if (pass.length() < 6 || pass.length() > 15) {
    //提示长度不准确
    AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度应为6~15"));
    return false;
  }

  // 创建一个正则表达式对象，按照上述密码要求
  // 这个正则表达式解释：
  // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
  QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*]{6,15}$");
  bool match = regExp.match(pass).hasMatch();
  if (!match) {
    //提示字符非法
    AddTipErr(TipErr::TIP_PWD_ERR, tr("不能包含非法字符"));
    return false;
    ;
  }

  DelTipErr(TipErr::TIP_PWD_ERR);

  return true;
}

bool RegisterDialog::checkEmailValid() {
  //验证邮箱的地址正则表达式
  auto email = ui->email_edit->text();
  // 邮箱地址的正则表达式
  QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
  bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
  if (!match) {
    //提示邮箱不正确
    AddTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
    return false;
  }

  DelTipErr(TipErr::TIP_EMAIL_ERR);
  return true;
}

bool RegisterDialog::checkVarifyValid() {
  auto pass = ui->varify_edit->text();
  if (pass.isEmpty()) {
    AddTipErr(TipErr::TIP_VERTIFY_ERR, tr("验证码不能为空"));
    return false;
  }

  DelTipErr(TipErr::TIP_VERTIFY_ERR);
  return true;
}

bool RegisterDialog::checkConfirmValid() {
  auto pass = ui->pass_edit->text();
  auto confirm = ui->confirm_edit->text();

  if (confirm.length() < 6 || confirm.length() > 15) {
    //提示长度不准确
    AddTipErr(TipErr::TIP_CONFIRM_ERR, tr("密码长度应为6~15"));
    return false;
  }

  // 创建一个正则表达式对象，按照上述密码要求
  // 这个正则表达式解释：
  // ^[a-zA-Z0-9!@#$%^&*]{6,15}$ 密码长度至少6，可以是字母、数字和特定的特殊字符
  QRegularExpression regExp("^[a-zA-Z0-9!@#$%^&*.]{6,15}$");
  bool match = regExp.match(confirm).hasMatch();
  if (!match) {
    //提示字符非法
    AddTipErr(TipErr::TIP_CONFIRM_ERR, tr("不能包含非法字符"));
    return false;
  }

  DelTipErr(TipErr::TIP_CONFIRM_ERR);

  if (pass != confirm) {
    //提示密码不匹配
    AddTipErr(TipErr::TIP_PWD_CONFIRM, tr("确认密码和密码不匹配"));
    return false;
  } else {
    DelTipErr(TipErr::TIP_PWD_CONFIRM);
  }
  return true;
}

void RegisterDialog::AddTipErr(TipErr te, QString tips) {
  tip_errs_[te] = tips;
  showTip(tips, false);
}

void RegisterDialog::DelTipErr(TipErr te) {
  tip_errs_.remove(te);
  if (tip_errs_.empty()) {
    ui->err_tip->clear();
    return;
  }

  showTip(tip_errs_.first(), false);
}

void RegisterDialog::ChangeTipPage() {
  countdown_timer_->stop();
  ui->stackedWidget->setCurrentWidget(ui->page_2);

  // 启动定时器，设置间隔为1000毫秒（1秒）
  countdown_timer_->start(1000);
}

void RegisterDialog::create_connection() {
  connect(HttpMgr::getinstance().get(), &HttpMgr::sig_reg_mod_finish, this,
          &RegisterDialog::slot_reg_mod_finish);
  ui->err_tip->clear();

  connect(ui->user_edit, &QLineEdit::editingFinished, this,
          [this]() { checkUserValid(); });

  connect(ui->email_edit, &QLineEdit::editingFinished, this,
          [this]() { checkEmailValid(); });

  connect(ui->pass_edit, &QLineEdit::editingFinished, this,
          [this]() { checkPassValid(); });

  connect(ui->confirm_edit, &QLineEdit::editingFinished, this,
          [this]() { checkConfirmValid(); });

  connect(ui->varify_edit, &QLineEdit::editingFinished, this,
          [this]() { checkVarifyValid(); });

  //连接ClickedLabel点击事件，改变其样式
  connect(ui->pass_visible, &ClickedLabel::clicked, this, [this]() {
    auto state = ui->pass_visible->GetCurState();
    if (state == ClickLbState::Normal) {
      ui->pass_edit->setEchoMode(QLineEdit::Password);
    } else {
      ui->pass_edit->setEchoMode(QLineEdit::Normal);
    }
    qDebug() << "Label was clicked!";
  });

  connect(ui->confirm_visible, &ClickedLabel::clicked, this, [this]() {
    auto state = ui->confirm_visible->GetCurState();
    if (state == ClickLbState::Normal) {
      ui->confirm_edit->setEchoMode(QLineEdit::Password);
    } else {
      ui->confirm_edit->setEchoMode(QLineEdit::Normal);
    }
    qDebug() << "Label was clicked!";
  });

  // 创建定时器
  countdown_timer_ = new QTimer(this);
  // 连接信号和槽
  connect(countdown_timer_, &QTimer::timeout, [this]() {
    if (countdown_ == 0) {
      countdown_timer_->stop();
      emit sigSwitchLogin();
      return;
    }
    countdown_--;
    auto str = QString("注册成功，%1 s后返回登录").arg(countdown_);
    ui->tip_lb->setText(str);
  });
}