#include "find_success_dlg.hpp"
#include "apply_friend_dialog.hpp"
#include "ui_findsuccessdlg.h"
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
FindSuccessDlg::FindSuccessDlg(QWidget *parent)
    : QDialog(parent), ui(new Ui::FindSuccessDlg), parent_(parent) {
  ui->setupUi(this);
  // 设置对话框标题
  setWindowTitle("添加");
  // 隐藏对话框标题栏
  setWindowFlags(windowFlags() | Qt::FramelessWindowHint);
  // 获取当前应用程序的路径，找到好友头像
  QString app_path = QCoreApplication::applicationDirPath();
  QString pix_path =
      QDir::toNativeSeparators(app_path + QDir::separator() + "static" +
                               QDir::separator() + "head_1.jpg");
  qDebug() << "pix_path:" << pix_path;
  ;
  //设置头像
  QPixmap head_pix(pix_path);
  head_pix = head_pix.scaled(ui->head_lb->size(), Qt::KeepAspectRatio,
                             Qt::SmoothTransformation);
  ui->head_lb->setPixmap(head_pix);
  // 初始化添加好友的按钮状态
  ui->add_friend_btn->SetState("normal", "hover", "press");
  this->setModal(true);
}

FindSuccessDlg::~FindSuccessDlg() {
  qDebug() << "FindSuccessDlg destruct";
  delete ui;
}

void FindSuccessDlg::set_search_info(std::shared_ptr<SearchInfo> si) {
  ui->name_lb->setText(si->_name);
  si_ = si;
}

void FindSuccessDlg::on_add_friend_btn_clicked() {
  // 隐藏搜索成功对话框
  this->hide();
  //弹出加好友界面
  auto applyFriend = new ApplyFriendDlg(parent_);
  applyFriend->set_search_info(si_);
  applyFriend->setModal(true);
  applyFriend->show();
}