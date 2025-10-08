#ifndef APPLY_FRIEND_PAGE_HPP
#define APPLY_FRIEND_PAGE_HPP
#include <QWidget>

namespace Ui {
class NewFriendApplyPage;
}
class ApplyInfo;
class AuthRsp;
class AddFriendApply;
class NewFriendApplyItem;

/**
 * @brief 新好友请求界面（展示好友申请列表并处理审核结果）
 */
class NewFriendApplyPage : public QWidget {
  Q_OBJECT

public:
  /**
   * @brief 构造函数
   * @param parent 父级窗口
   */
  explicit NewFriendApplyPage(QWidget *parent = nullptr);

  /**
   * @brief 析构函数
   */
  ~NewFriendApplyPage();

  /**
   * @brief 添加新的好友申请项
   * @param apply 好友申请信息
   */
  void add_new_apply(std::shared_ptr<AddFriendApply> apply);

  /**
   * @brief 登录时，加载服务器发送过来的好友申请列表
   */
  void load_apply_list();

protected:
  /**
   * @brief 重写绘制事件，确保样式表生效
   * @param event 绘制事件
   */
  void paintEvent(QPaintEvent *event) override;

private:
  Ui::NewFriendApplyPage *ui; ///< UI界面指针
  std::unordered_map<int, NewFriendApplyItem *>
      unauth_items_; ///< 未审核好友请求项（uid->item),即 暂未同意好友请求的项

public slots:
  /**
   * @brief 处理好友认证响应
   * @param auth_rsp 认证结果响应
   */
  void slot_handle_auth_rsp(std::shared_ptr<AuthRsp>);

  /**
   * @brief 显示好人认证对话框，显示好友申请信息并决定是否同意对方的申请
   * @param apply_info 好友申请信息
   */
  void slot_show_auth_friend_dlg(std::shared_ptr<ApplyInfo> apply_info);

signals:
  void sig_recv_new_friend_apply();

  /**
   * @brief 通知外部是否显示搜索框
   * @param show 是否显示
   */
  void sig_show_search(bool);
};

#endif // APPLY_FRIEND_PAGE_HPP