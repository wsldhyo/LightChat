#ifndef CONTACT_USER_LIST_HPP
#define CONTACT_USER_LIST_HPP
#include <QListWidget>
struct AuthRsp;
struct AuthInfo;
struct UserInfo;

class ContactUserItem;
/**
 * @brief 联系人列表组件（继承自 QListWidget）
 *
 * 用于展示联系人、分组提示（GroupTipItem）、以及“新的朋友”入口。
 */
class ContactUserList : public QListWidget {
  Q_OBJECT
public:
  ContactUserList(QWidget *parent = nullptr);

  /// 显示或隐藏“新的朋友”条目右上角的红点
  void ShowRedPoint(bool bshow = true);

protected:
  /// 事件过滤器：处理鼠标进入、离开和滚轮事件
  bool eventFilter(QObject *watched, QEvent *event) override;

private:
  /// 初始化并加载联系人列表数据
  void add_contact_user_list();


public slots:
  /// 处理点击条目的逻辑
  void slot_item_clicked(QListWidgetItem *item);

  // 对方处理完好友申请后的处理
  void slot_recv_friend_auth(std::shared_ptr<AuthInfo> auth_info);
  /// 处理对方好友请求后, 对服务器回包的处理
  void slot_friend_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp);

signals:
  /// 通知界面加载更多联系人（滚动到底部时触发）
  void sig_loading_contact_user();
  /// 切换到申请好友页面
  void sig_switch_apply_friend_page();
  /// 切换到好友信息页面
  void sig_switch_friend_info_page(std::shared_ptr<UserInfo> ui);

private:
  void create_connection();

  ContactUserItem *add_friend_item_; ///< “新的朋友”条目
  QListWidgetItem *group_item_;      ///< “联系人”分组提示条目

  bool load_pending_;
};

#endif // CONTACT_USER_LIST_HPP
