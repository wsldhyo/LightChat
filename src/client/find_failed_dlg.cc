#include "find_failed_dlg.hpp"
#include "ui_findfaileddlg.h"
#include <QDebug>

FindFailedDlg::FindFailedDlg(QWidget *parent)
    : QDialog(parent), ui(new Ui::FindFailedDlg) {
  ui->setupUi(this);
  // 设置对话框标题
  setWindowTitle("添加");
  // 隐藏对话框标题栏
  setWindowFlags(windowFlags() | Qt::FramelessWindowHint); // 隐藏边框
  this->setObjectName("FindFailedDlg");
  ui->fail_sure_btn->SetState("normal", "hover", "press");
  this->setModal(true); // 模态
}

FindFailedDlg::~FindFailedDlg() {
  qDebug() << "Find FailDlg destruct";
  delete ui;
}
// 点击失败框的确认按钮，隐藏对话框
void FindFailedDlg::on_fail_sure_btn_clicked() { this->hide(); }