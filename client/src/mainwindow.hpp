#ifndef MAIN_WINDOW_HPP
#define MAIN_WINDOW_HPP
#include <QMainWindow>
#include <QDialog>
#include "ui_mainwindow.h"
#include "login_dlg.hpp"
#include "register_dlg.hpp"
#include "reset_pwd_dlg.hpp"
class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* _parent = nullptr);

public slots:
    void slot_switch_register_dlg();
    void slot_switch_login_dlg();
    void slot_switch_reset_pwd_dlg();
    void slog_switch_login_from_reset_pwd_dlg();
private:
    void create_connection();


    Ui::MainWindow* window_;
    LoginDlg* login_dlg_;
    RegisterDlg* register_dlg_;
    ResetPwdDlg* reset_pwd_dlg_;
};

#endif