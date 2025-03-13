#include "mainwindow.hpp"
#include "login_dlg.hpp"
#include "register_dlg.hpp"
#include <qlocale.h>
#include <qnamespace.h>
MainWindow::MainWindow(QWidget *_parent /*nullptr*/)
    : QMainWindow(_parent), window_(new Ui::MainWindow) {

  window_->setupUi(this);
  login_dlg_ = new LoginDlg();
  setCentralWidget(login_dlg_);
  login_dlg_->setWindowFlags(Qt::FramelessWindowHint | Qt::CustomizeWindowHint);
  login_dlg_->show();

  register_dlg_ = new RegisterDlg(this);
  register_dlg_->hide();

  create_connection();
}

void MainWindow::create_connection() {
  connect(login_dlg_, &LoginDlg::switch_register_dlg, this,
          &MainWindow::slot_switch_register_dlg);
}

void MainWindow::slot_switch_register_dlg()
{

    login_dlg_->hide();
    register_dlg_->show(); 
}