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
  // SlotSwitchChat();
}

MainWindow::~MainWindow() { delete ui; }

/**
 @brief 点击登录界面的注册按钮时，切换到注册界面：
 创建注册界面，将其显示，并隐藏登录界面
 @param void
 @return void
*/
void MainWindow::SlotSwitchReg() {
  reg_dlg_ = new RegisterDialog(this);
  reg_dlg_->hide();

  reg_dlg_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);

  //连接注册界面返回登录信号
  connect(reg_dlg_, &RegisterDialog::sigSwitchLogin, this,
          &MainWindow::SlotSwitchLogin);
  setCentralWidget(reg_dlg_);
  login_dlg_->hide();
  reg_dlg_->show();
}

//从注册界面返回登录界面
void MainWindow::SlotSwitchLogin() {
  //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
  login_dlg_ = new LoginDialog(this);
  login_dlg_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  setCentralWidget(login_dlg_);

  reg_dlg_->hide();
  login_dlg_->show();
  //连接登录界面注册信号
  connect(login_dlg_, &LoginDialog::switchRegister, this,
          &MainWindow::SlotSwitchReg);
  //连接登录界面忘记密码信号
  // connect( login_dlg_, &LoginDialog::switchReset, this,
  // &MainWindow::SlotSwitchReset);
}

//从重置界面返回登录界面
void MainWindow::SlotSwitchLogin2() {
  //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
  login_dlg_ = new LoginDialog(this);
  login_dlg_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  setCentralWidget(login_dlg_);

  reset_dlg_->hide();
  login_dlg_->show();
  //连接登录界面忘记密码信号
  connect(login_dlg_, &LoginDialog::switchReset, this,
          &MainWindow::SlotSwitchReset);
  //连接登录界面注册信号
  connect(login_dlg_, &LoginDialog::switchRegister, this,
          &MainWindow::SlotSwitchReg);
}

void MainWindow::SlotSwitchReset() {
  //创建一个CentralWidget, 并将其设置为MainWindow的中心部件
  reset_dlg_ = new ResetDialog(this);
  reset_dlg_->setWindowFlags(Qt::CustomizeWindowHint | Qt::FramelessWindowHint);
  setCentralWidget(reset_dlg_);

  login_dlg_->hide();
  reset_dlg_->show();
  //注册返回登录信号和槽函数
  connect(reset_dlg_, &ResetDialog::switchLogin, this,
          &MainWindow::SlotSwitchLogin2);
}

void MainWindow::SlotSwitchChat() {
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
  connect(login_dlg_, &LoginDialog::switchRegister, this,
          &MainWindow::SlotSwitchReg);
  //连接登录界面注册信号
  connect(login_dlg_, &LoginDialog::switchRegister, this,
          &MainWindow::SlotSwitchReg);
  //连接登录界面忘记密码信号
  connect(login_dlg_, &LoginDialog::switchReset, this,
          &MainWindow::SlotSwitchReset);
  //连接创建聊天界面信号
  connect(TcpMgr::getinstance().get(), &TcpMgr::sig_switch_chatdlg, this,
          &MainWindow::SlotSwitchChat);
}
