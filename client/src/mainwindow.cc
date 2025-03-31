#include "mainwindow.hpp"
#include "login_dlg.hpp"
#include "register_dlg.hpp"
#include "tcp_manager.hpp"
MainWindow::MainWindow(QWidget *_parent /*nullptr*/)
    : QMainWindow(_parent), window_(new Ui::MainWindow) {

  window_->setupUi(this);
  login_dlg_ = new LoginDlg(this);
  setCentralWidget(login_dlg_);
  login_dlg_->setWindowFlags(Qt::FramelessWindowHint | Qt::CustomizeWindowHint);

  create_connection();
  emit TcpManager::get_instance()->sig_switch_chat_dlg();
}

void MainWindow::create_connection() {
  connect(login_dlg_, &LoginDlg::sig_switch_register_dlg, this,
          &MainWindow::slot_switch_register_dlg);
  // 连接登录界面忘记密码信号
  connect(login_dlg_, &LoginDlg::sig_switch_reset_pwd, this,
          &MainWindow::slot_switch_reset_pwd_dlg);

  connect(TcpManager::get_instance().get(), &TcpManager::sig_switch_chat_dlg,
          this, &MainWindow::slot_switch_chat_dlg);
}

void MainWindow::slot_switch_register_dlg() {

  register_dlg_ = new RegisterDlg(this);
  register_dlg_->hide();
  register_dlg_->setWindowFlags(Qt::CustomizeWindowHint |
                                Qt::FramelessWindowHint);
  connect(register_dlg_, &RegisterDlg::sig_switch_login_page, this,
          &MainWindow::slot_switch_login_dlg);
  login_dlg_->hide();
  register_dlg_->show();
}

void MainWindow::slot_switch_chat_dlg() {
  chat_dlg_ = new ChatDlg();
  chat_dlg_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  setCentralWidget(chat_dlg_);
  chat_dlg_->show();
  login_dlg_->hide();
  this->setMinimumSize(QSize(1050, 900));
  this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

// 从注册界面返回登录界面
void MainWindow::slot_switch_login_dlg() {
  // 创建一个CentralWidget, 并将其设置为MainWindow的中心部件
  login_dlg_ = new LoginDlg(this);
  login_dlg_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  setCentralWidget(login_dlg_);
  register_dlg_->hide();
  login_dlg_->show();
  // 连接登录界面忘记密码信号
  connect(login_dlg_, &LoginDlg::sig_switch_reset_pwd, this,
          &MainWindow::slot_switch_reset_pwd_dlg);
  // 连接登录界面注册信号
  connect(login_dlg_, &LoginDlg::sig_switch_register_dlg, this,
          &MainWindow::slot_switch_register_dlg);
}

void MainWindow::slot_switch_reset_pwd_dlg() {
  // 创建一个CentralWidget, 并将其设置为MainWindow的中心部件
  reset_pwd_dlg_ = new ResetPwdDlg(this);
  reset_pwd_dlg_->setWindowFlags(Qt::CustomizeWindowHint |
                                 Qt::FramelessWindowHint);
  setCentralWidget(reset_pwd_dlg_);
  login_dlg_->hide();
  reset_pwd_dlg_->show();
  // 注册返回登录信号和槽函数
  connect(reset_pwd_dlg_, &ResetPwdDlg::sig_switch_login_page, this,
          &MainWindow::slot_switch_login_from_reset_pwd_dlg);
}

void MainWindow::slot_switch_login_from_reset_pwd_dlg() {
  // 创建一个CentralWidget, 并将其设置为MainWindow的中心部件
  login_dlg_ = new LoginDlg(this);
  login_dlg_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  setCentralWidget(login_dlg_);
  reset_pwd_dlg_->hide();
  login_dlg_->show();
  // 连接登录界面忘记密码信号
  connect(login_dlg_, &LoginDlg::sig_switch_reset_pwd, this,
          &MainWindow::slot_switch_reset_pwd_dlg);
  // 连接登录界面注册信号
  connect(login_dlg_, &LoginDlg::sig_switch_register_dlg, this,
          &MainWindow::slot_switch_register_dlg);
}