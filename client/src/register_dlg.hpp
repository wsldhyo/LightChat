 #ifndef REGISTER_DLG_HPP
#define REGISTER_DLG_HPP
#include <QDialog>
#include <qobjectdefs.h>
#include "ui_register_dlg.h"
#include <QString>
class RegisterDlg : public QDialog
{
    Q_OBJECT
public:
    RegisterDlg(QWidget* _parent);

private slots:
    void slot_get_code_clicked();
private:
    void create_connection();
    void show_tip(QString _str, bool _ok);
    Ui::RegisterDlg* ui_;
};

#endif