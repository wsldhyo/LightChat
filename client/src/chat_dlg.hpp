#ifndef CHAT_DIALOG_HPP
#define CHAT_DIALOG_HPP
#include <QDialog>
#include "ui_chat_dlg.h"

class ChatDlg : public QDialog
{
    Q_OBJECT
public:
    ChatDlg(QWidget* _parent = nullptr);
    ~ChatDlg();
private:
    Ui::ChatDialog* ui_;
};

#endif