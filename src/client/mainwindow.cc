#include "mainwindow.hpp"
#include "chat_dialog.hpp"
#include "login_dialog.hpp"
#include "register_dialog.hpp"
#include "reset_dialog.hpp"
#include "tcp_manager.hpp"
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow), chat_dlg_(nullptr) {
  ui->setupUi(this);
  login_dlg_ = new LoginDialog(this);
  reg_dlg_ = new RegisterDialog(this);
  reg_dlg_->hide();
  setCentralWidget(login_dlg_);
  login_dlg_->show();
  create_connection();
  // slot_switch_chat();
}

MainWindow::~MainWindow() { delete ui; }

/**
 @brief 点击登录界面的注册按钮时，切换到注册界面：
 创建注册界面，将其显示，并隐藏登录界面
 @param void
 @return void
*/
void MainWindow::slot_switch_reg() {
  reg_dlg_ = new RegisterDialog(this);
  reg_dlg_->hide();

  reg_dlg_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

  //连接注册界面返回登录信号
  connect(reg_dlg_, &RegisterDialog::sig_switch_login, this,
          &MainWindow::slot_switch_login);
  setCentralWidget(reg_dlg_);
  login_dlg_->hide();
  reg_dlg_->show();
}

//从注册界面返回登录界面
void MainWindow::slot_switch_login() {
  //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
  login_dlg_ = new LoginDialog(this);
  login_dlg_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  setCentralWidget(login_dlg_);

  reg_dlg_->hide();
  login_dlg_->show();
  //连接登录界面注册信号
  connect(login_dlg_, &LoginDialog::switch_register, this,
          &MainWindow::slot_switch_reg);
  //连接登录界面忘记密码信号
  // connect( login_dlg_, &LoginDialog::switch_reset, this,
  // &MainWindow::slot_switch_reset);
}

//从重置界面返回登录界面
void MainWindow::slot_switch_login2() {
  //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
  login_dlg_ = new LoginDialog(this);
  login_dlg_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  setCentralWidget(login_dlg_);

  reset_dlg_->hide();
  login_dlg_->show();
  //连接登录界面忘记密码信号
  connect(login_dlg_, &LoginDialog::switch_reset, this,
          &MainWindow::slot_switch_reset);
  //连接登录界面注册信号
  connect(login_dlg_, &LoginDialog::switch_register, this,
          &MainWindow::slot_switch_reg);
}

void MainWindow::slot_switch_reset() {
  //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
  reset_dlg_ = new ResetDialog(this);
  reset_dlg_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  setCentralWidget(reset_dlg_);

  login_dlg_->hide();
  reset_dlg_->show();
  //注册返回登录信号和槽函数
  connect(reset_dlg_, &ResetDialog::sig_switch_login, this,
          &MainWindow::slot_switch_login2);
}

void MainWindow::slot_switch_chat() {
  chat_dlg_ = new ChatDialog();
  chat_dlg_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  setCentralWidget(chat_dlg_);
  chat_dlg_->show();
  login_dlg_->hide();
  this->setMinimumSize(QSize(1050, 900));
  this->setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);
}

void MainWindow::create_connection() {
  //创建和注册消息的链接
  connect(login_dlg_, &LoginDialog::switch_register, this,
          &MainWindow::slot_switch_reg);
  //连接登录界面注册信号
  connect(login_dlg_, &LoginDialog::switch_register, this,
          &MainWindow::slot_switch_reg);
  //连接登录界面忘记密码信号
  connect(login_dlg_, &LoginDialog::switch_reset, this,
          &MainWindow::slot_switch_reset);
  //连接创建聊天界面信号
  connect(TcpMgr::get_instance().get(), &TcpMgr::sig_switch_chatdlg, this,
          &MainWindow::slot_switch_chat);
}
