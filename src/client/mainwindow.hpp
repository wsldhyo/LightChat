#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include <QMainWindow>
#include "client_constant.hpp"
/******************************************************************************
 *
 * @file       mainwindow.h
 * @brief      主界面功能 Function
 *
 * @author     wsldhyo
 * @date       2025/08/18
 * @history
 *****************************************************************************/
namespace Ui {
class MainWindow;
}


class LoginDialog;
class RegisterDialog;
class ResetDialog;
class ChatDialog;

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
public slots:
  void slot_switch_reg();
  void slot_switch_login();
  void slot_switch_login_from_reset();
  void slot_switch_reset();
  // 登录成功后，切换到聊天界面
  void slot_switch_chat();
  void slot_switch_login_from_chat();
  void slot_notify_connection_closed();

private:
  void create_connection();
  void offline();

  Ui::MainWindow *ui;
  LoginDialog *login_dlg_;
  RegisterDialog *reg_dlg_;
  ResetDialog *reset_dlg_;
  ChatDialog *chat_dlg_;
  UIStatus ui_status_;
};

#endif // MAINWINDOW_H
