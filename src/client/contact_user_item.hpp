#ifndef CONTACT_USER_ITEM_HPP
#define CONTACT_USER_ITEM_HPP
#include "list_item_base.hpp"
class UserInfo;
namespace Ui {
class ContactUserItem;
}
struct AuthInfo;
struct AuthRsp;

/**
 * @brief 联系人条目（显示头像、昵称等）
 * 
 * 可表示单个联系人，也可表示好友请求。
 */
class ContactUserItem : public ListItemBase {
  Q_OBJECT

public:
  explicit ContactUserItem(QWidget *parent = nullptr);
  ~ContactUserItem();

  QSize sizeHint() const override;

  /// 设置为 AuthInfo 类型的联系人
  void SetInfo(std::shared_ptr<AuthInfo> auth_info);
  /// 设置为 AuthRsp 类型的联系人
  void SetInfo(std::shared_ptr<AuthRsp> auth_rsp);
  /// 设置为 普通联系人（id、名称、头像）
  void SetInfo(int uid, QString name, QString icon);

  /// 显示/隐藏右上角的红点
  void ShowRedPoint(bool show = false);

  std::shared_ptr<UserInfo>const get_user_info()const;

private:
  Ui::ContactUserItem *ui;          ///< UI 界面指针
  std::shared_ptr<UserInfo> _info;  ///< 存储联系人信息
};


#endif // CONTACT_USER_ITEM