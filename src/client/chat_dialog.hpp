#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include "client_constant.hpp"
#include <QDialog>
#include <QLabel>
#include <QList>
#include <QListWidgetItem>
#include <memory>

namespace Ui {
class ChatDialog;
}

class StateWidget;
/**
 * @brief 聊天对话框类，包含聊天会话列表ChatUserList和聊天页面ChatPage
 * 
 */
class ChatDialog : public QDialog {
  Q_OBJECT

public:
  explicit ChatDialog(QWidget *parent = nullptr);
  ~ChatDialog();

protected:
  // bool eventFilter(QObject *watched, QEvent *event) override;

  // void handleGlobalMousePress(QMouseEvent *event);
  // void CloseFindDlg();
  // void UpdateChatMsg(std::vector<std::shared_ptr<TextChatData>> msgdata);

private:
  /**
   * @brief 设置搜索框，输入时在最右边显示删除按钮
   * @todo 放到CustomizeEdit里
   *
   */
  void set_search_edit();

  void add_chat_user_list();

  void clear_label_state(StateWidget *lb);

  void add_lb_group(StateWidget *lb);
  // void loadMoreChatUser();
  // void loadMoreConUser();
  // void SetSelectChatItem(int uid = 0);
  // void SetSelectChatPage(int uid = 0);
  void show_search(bool bsearch = false);

  void create_connection();
public slots:
  void slot_loading_chat_user();
  void slot_side_chat();
  void slot_side_contact();
  // void slot_side_setting();
  void slot_text_changed(const QString &str);
  // void slot_focus_out();
  // void slot_loading_contact_user();
  // void slot_switch_apply_friend_page();
  // void slot_friend_info_page(std::shared_ptr<UserInfo> user_info);
  // void slot_show_search(bool show);
  // void slot_apply_friend(std::shared_ptr<AddFriendApply> apply);
  // void slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info);
  // void slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp);
  // void slot_jump_chat_item(std::shared_ptr<SearchInfo> si);
  // void slot_jump_chat_item_from_infopage(std::shared_ptr<UserInfo> ui);
  // void slot_item_clicked(QListWidgetItem *item);
  // void slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg);
  // void slot_append_send_chat_msg(std::shared_ptr<TextChatData> msgdata);
private slots:

private:
  Ui::ChatDialog *ui;
  ChatUIMode mode_; //根据sidebar显示不同的界面：如联系人界面、会话界面
  ChatUIMode state_; // 不同mode下的搜索框状态
  bool b_loading_;
  QList<StateWidget *> lb_list_;
};

#endif // CHATDIALOG_H
