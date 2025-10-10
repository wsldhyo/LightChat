#ifndef CHAT_USER_WID_HPP
#define CHAT_USER_WID_HPP
#include "chat_user_list.hpp"
#include "list_item_base.hpp"
#include <memory>
struct UserInfo;
struct FriendInfo;
struct TextChatData;
namespace Ui {
class ChatUserWid;
}
/**
 * @brief
 * 自定义聊天会话列表的列表项，依次显示头像，用户名、最近消息以及最近消息的时间
 *
 */
class ChatUserWid : public ListItemBase {
  Q_OBJECT

public:
  explicit ChatUserWid(QWidget *parent = nullptr);
  ~ChatUserWid();

  QSize sizeHint() const override {
    return QSize(250, 70); // 返回自定义的尺寸
  }

  void set_friend_info(std::shared_ptr<UserInfo> user_info);
  void set_friend_info(std::shared_ptr<FriendInfo> user_info);

  std::shared_ptr<UserInfo> const get_user_info() const;

  // 更新当前会话项中的最后一条消息，用于在聊天会话列表中展示。
  // 登录时可能收到多条消息。
  void update_last_msg(std::vector<std::shared_ptr<TextChatData>> msgs);

private:
  Ui::ChatUserWid *ui;
  std::shared_ptr<UserInfo> friend_info_;
};
#endif