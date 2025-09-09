#ifndef NEW_FRIEND_APPLY_ITEM_HPP
#define NEW_FRIEND_APPLY_ITEM_HPP
#include "list_item_base.hpp"

namespace Ui {
class NewFriendApplyItem;
}

class ApplyInfo;

/**
 * @brief 单个好友请求列表项（显示头像、昵称、描述、按钮等）
 */
class NewFriendApplyItem : public ListItemBase {
  Q_OBJECT

public:
  /**
   * @brief 构造函数
   * @param parent 父窗口
   */
  explicit NewFriendApplyItem(QWidget *parent = nullptr);

  /**
   * @brief 析构函数
   */
  ~NewFriendApplyItem();

  /**
   * @brief 设置好友申请信息
   * @param apply_info 好友申请数据
   */
  void set_info(std::shared_ptr<ApplyInfo> apply_info);

  /**
   * @brief 控制“添加好友”按钮是否显示，如果已同意好友请求则隐藏添加按钮
   * @param bshow 是否显示
   */
  void show_add_btn(bool bshow);

  /**
   * @brief 返回该申请的用户ID
   * @return 用户ID
   */
  int get_uid();

  /**
   * @brief 自定义列表项尺寸
   */
  QSize sizeHint() const override { return QSize(250, 80); }

private:
  Ui::NewFriendApplyItem *ui;
  std::shared_ptr<ApplyInfo> apply_info_; ///< 申请数据
  bool added_;                            ///< 是否已添加成功

signals:
  /**
   * @brief 发起审核好友信号（点击添加按钮触发）
   * @param apply_info 当前申请信息
   */
  void sig_auth_friend(std::shared_ptr<ApplyInfo> apply_info);
};

#endif // NEW_FRIEND_APPLY_ITEM_HPP