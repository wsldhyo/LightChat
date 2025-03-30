#include "chat_dlg.hpp"

ChatDlg::ChatDlg(QWidget *_parent /*nullptr*/)
    : QDialog(_parent), ui_(new Ui::ChatDialog) {
        ui_->add_btn->set_state("normal", "hover", "press");
    }

ChatDlg::~ChatDlg() { delete ui_; }