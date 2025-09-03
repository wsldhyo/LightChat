#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "logindialog.hpp"
#include "registerdialog.hpp"
#include "resetdialog.hpp"
#include "ui_mainwindow.h"
#include <QMainWindow>
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

enum UIStatus { LOGIN_UI, REGISTER_UI, RESET_UI, CHAT_UI };

class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = nullptr);
  ~MainWindow();
public slots:
  void SlotSwitchReg();
  void SlotSwitchLogin();
  void SlotSwitchLogin2();
  void SlotSwitchReset();

private:
  void create_connection();

  Ui::MainWindow *ui;
  LoginDialog *login_dlg_;
  RegisterDialog *reg_dlg_;
  ResetDialog *reset_dlg_;
};

#endif // MAINWINDOW_H
