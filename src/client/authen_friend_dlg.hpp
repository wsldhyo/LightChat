#ifndef AUTHEN_FRIEND_DLG_HPP
#define AUTHEN_FRIEND_DLG_HPP
#include "client_constant.hpp"
#include <QDialog>
#include <QMap>
class ClickedLabel;
class FriendLabel;
class ApplyInfo;
namespace Ui {
class AuthenFriendDlg;
}

// 基本同申请页面（包括函数含义）
class AuthenFriendDlg : public QDialog {
  Q_OBJECT

public:
  explicit AuthenFriendDlg(QWidget *parent = nullptr);
  ~AuthenFriendDlg();

  void init_tip_lbs();
  void add_tip_lbs(ClickedLabel *, QPoint cur_point, QPoint &next_point,
                   int text_width, int text_height);
  void set_apply_info(std::shared_ptr<ApplyInfo> apply_info);

public slots:
  //显示更多label标签
  void slot_show_more_label();
  //输入label按下回车触发将标签加入展示栏
  void slot_label_enter();
  //点击关闭，移除展示栏好友便签
  void slot_remove_friend_label(QString);
  //通过点击tip实现增加和减少好友便签
  void slot_change_friend_label_by_tip(QString, ClickLbState);
  //输入框文本变化显示不同提示
  void slot_label_text_change(const QString &text);
  //输入框输入完成
  void slot_label_edit_finished();
  //输入标签显示提示框，点击提示框内容后添加好友便签
  void slot_add_firend_label_by_click_tip(QString text);
  //处理确认回调
  void slot_apply_sure();
  //处理取消回调
  void slot_apply_cancel();

protected:
  bool eventFilter(QObject *obj, QEvent *event) override;

private:
  void reset_labels();

  void add_label(QString name);

private:
  std::shared_ptr<ApplyInfo> apply_info_;
  Ui::AuthenFriendDlg *ui;
  //已经创建好的标签
  QMap<QString, ClickedLabel *> add_labels_;
  std::vector<QString> add_label_keys_;
  QPoint label_point_;
  //用来在输入框显示添加新好友的标签
  QMap<QString, FriendLabel *> friend_labels_;
  std::vector<QString> friend_label_keys_;
  std::vector<QString> tip_data_;
  QPoint tip_cur_point_;
};
#endif // AUTHEN_FRIEND_DLG_HPP