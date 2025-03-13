#ifndef LOGIN_DLG_HPP
#define LOGIN_DLG_HPP
#include "ui_logindlg.h"
#include <QDialog>
#include <qobjectdefs.h>

class LoginDlg : public QDialog
{
    Q_OBJECT
public:
    LoginDlg(QWidget* _parent = nullptr);

private:
    void create_connection();
signals:
    void switch_register_dlg();
private:
    Ui::LoginDlg* ui_;
};

#endif