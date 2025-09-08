#ifndef APPLY_FRIEND_DIALOG_HPP
#define APPLY_FRIEND_DIALOG_HPP
#include "clicked_label.hpp"
#include "friend_label.hpp"
#include "user_data.hpp"
#include <QDialog>
#include <QMap>
#include <memory>
#include <vector>

namespace Ui {
class ApplyFriendDlg;
}

/**
 * @brief 好友申请对话框，可以设置标签、备注
 *
 * 功能：
 * 1. 输入昵称、备注信息
 * 2. 为好友添加标签（支持输入、自定义、推荐标签）
 * 3. 标签区域动态布局（支持换行、删除、重新排版）
 * 4. 标签推荐提示（输入前缀 -> 提示候选 -> 点击添加）
 */
class ApplyFriendDlg : public QDialog {
  Q_OBJECT

public:
  explicit ApplyFriendDlg(QWidget *parent = nullptr);
  ~ApplyFriendDlg();

  /// 初始化标签提示区（推荐标签）
  void init_tip_lbs();

  /// 向推荐标签区域添加一个标签控件
  void add_tip_lbs(ClickedLabel *, QPoint cur_point, QPoint &next_point,
                   int text_width, int text_height);

  /// 事件过滤器（用于滚动区域悬停时显示/隐藏滚动条）
  bool eventFilter(QObject *obj, QEvent *event) override;

  /// 设置搜索信息（申请人昵称、备注）
  void set_search_info(std::shared_ptr<SearchInfo> si);

private:
  /// 重新排版已添加的好友标签
  void reset_labels();

  /// 添加一个新的好友标签到展示区
  void add_label(QString name);

public slots:
  /// 显示更多推荐标签
  void slot_show_more_label();

  /// 输入框回车后，将输入内容作为标签添加
  void slot_label_enter();

  /// 点击标签上的关闭按钮 -> 移除标签
  void slot_remove_friend_label(QString);

  /// 点击推荐标签 -> 将推荐标签添加到好友标签或从好友标签中移除
  void slot_change_friend_label_by_tip(QString, ClickLbState);

  /// 输入框内容变化 -> 显示不同的推荐提示
  void slot_label_text_change(const QString &text);

  /// 输入框编辑完成 -> 隐藏提示框
  void slot_label_edit_finished();

  /// 点击提示框中的内容 -> 添加好友标签
  void slot_add_firend_label_by_click_tip(QString text);

  /// 点击“确认”按钮
  void slot_apply_sure();

  /// 点击“取消”按钮
  void slot_apply_cancel();

private:
  Ui::ApplyFriendDlg *ui;

  /// 推荐标签区域已有的标签（提示标签）
  QMap<QString, ClickedLabel *> add_labels_;
  std::vector<QString> add_label_keys_;

  /// 好友标签展示区中已添加的标签
  QMap<QString, FriendLabel *> friend_labels_;
  std::vector<QString> friend_label_keys_;

  /// 输入框提示候选数据（已有FriendLabel的文本数据）
  std::vector<QString> tip_data_;

  /// 已选择标签的排版位置
  QPoint label_point_;
  /// 下一个待添加“推荐标签”的左上角绘制位置
  QPoint tip_cur_point_;

  /// 搜索信息（申请人相关）
  std::shared_ptr<SearchInfo> si_;
};
#endif // APPLY_FRIEND_DIALOG_HPP