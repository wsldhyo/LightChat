#ifndef CHAT_DIALOG_HPP
#define CHAT_DIALOG_HPP
#include <QDialog>
#include "ui_chat_dlg.h"
#include "../common/constant.hpp"
class QAction;
class ChatDlg : public QDialog
{
    Q_OBJECT
public:
    ChatDlg(QWidget* _parent = nullptr);
    ~ChatDlg();
private:
    void create_connection();
    void show_search(bool _bsearch);
    Ui::ChatDialog* ui_;
    QAction* search_action_;
    QAction* clear_action_;
    ChatUiMode mode_;
    ChatUiMode state_;
    bool b_loading;

};

#endif