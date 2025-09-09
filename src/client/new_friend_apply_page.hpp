#ifndef APPLY_FRIEND_PAGE_HPP
#define APPLY_FRIEND_PAGE_HPP
#include <QWidget>

namespace Ui {
class NewFriendApplyPage;
}
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

protected:
  /**
   * @brief 重写绘制事件，确保样式表生效
   * @param event 绘制事件
   */
  void paintEvent(QPaintEvent *event) override;

private:
  /**
   * @brief 加载好友申请列表
   */
  void load_apply_list();

  Ui::NewFriendApplyPage *ui; ///< UI界面指针
  std::unordered_map<int, NewFriendApplyItem *>
      unauth_items_; ///< 未审核好友请求项（uid->item),即 暂未同意好友请求的项

public slots:
  /**
   * @brief 处理好友认证响应
   * @param auth_rsp 认证结果响应
   */
  void slot_auth_rsp(std::shared_ptr<AuthRsp>);

signals:
  /**
   * @brief 通知外部是否显示搜索框
   * @param show 是否显示
   */
  void sig_show_search(bool);
};

#endif // APPLY_FRIEND_PAGE_HPP