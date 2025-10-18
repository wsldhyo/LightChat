#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include "client_constant.hpp"
#include <QDialog>
#include <QLabel>
#include <QList>
#include <QListWidgetItem>
#include <memory>
class AddFriendApply;
struct AuthInfo;
struct AuthRsp;
class SearchInfo;
struct UserInfo;
struct TextChatData;
struct TextChatMsg;
namespace Ui {
class ChatDialog;
}
class QTimer;

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
  bool eventFilter(QObject *watched, QEvent *event) override;

  // void CloseFindDlg();
  // void UpdateChatMsg(std::vector<std::shared_ptr<TextChatData>> msgdata);

private:
  void handle_global_mouse_press(QMouseEvent *event);
  /**
   * @brief 设置搜索框，输入时在最右边显示删除按钮
   * @todo 放到CustomizeEdit里
   *
   */
  void set_search_edit();

  void add_chat_user_list();

  void clear_label_state(StateWidget *lb);

  void add_lb_group(StateWidget *lb);
  // 聊天会话列表向下滚动时，加载更多会话项
  void load_more_chat_user();
  // 联系人列表向下滚动时，加载更多联系人项
  void load_more_contact_user();
  void set_select_chat_item(int uid = 0);
  void set_select_chat_page(int uid = 0);
  void show_search(bool bsearch = false);

  void create_connection();
  void update_chat_msg(std::vector<std::shared_ptr<TextChatData>> msgdata);
public slots:
  void slot_loading_chat_user();
  void slot_loading_contact_user();
  void slot_side_chat();
  void slot_side_contact();
  // void slot_side_setting();
  void slot_text_changed(const QString &str);
  // void slot_focus_out();
  // 切换到好友详细信息页面
  void slot_switch_friend_info_page(std::shared_ptr<UserInfo> user_info);

  void slot_switch_apply_friend_page();
  // void slot_show_search(bool show);
  void slot_handle_friend_apply(std::shared_ptr<AddFriendApply> apply);
  // 对方处理完好友申请后的处理 （将对方加入到聊天会话列表）
  void slot_recv_friend_auth(std::shared_ptr<AuthInfo> auth_info);
  /// 处理对方好友请求后, 对服务器回包的处理（将对方加入到聊天会话列表）
  void slot_friend_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp);
  void slot_switch_chat_item(std::shared_ptr<SearchInfo> si);
  // void slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg);
  void slot_show_friend_apply_red_point();
  // 从好友详细信息页面跳转到与该好友的聊天会话页面
  void slot_switch_chat_item_from_infopage(std::shared_ptr<UserInfo> user_info);
  // 对界面内列表item的点击处理
  void slot_item_clicked(QListWidgetItem *item);
  // 添加聊天消息到本地，以便切换会话视图时可以切换消息记录
  void slot_append_send_chat_msg(std::shared_ptr<TextChatData> msgdata);
  // 收到他人的消息
  void slot_recv_text_msg(std::shared_ptr<TextChatMsg> msg);
private slots:

private:
  Ui::ChatDialog *ui;
  ChatUIMode mode_; //根据sidebar显示不同的界面：如联系人界面、会话界面
  ChatUIMode state_; // 不同mode下的搜索框状态
  bool b_loading_;
  // 侧边栏标签列表，一次仅有一个激活并显示对应标签的列表（会话列表、联系人列表）
  QList<StateWidget *> lb_list_;

  QMap<int, QListWidgetItem *>
      chat_items_added_; // 已经加入到聊天会话列表的会话项

  int cur_chat_uid_; /// 当前激活的聊天视图的uid（展示的是哪一个用户的聊天记录）
  QWidget *last_widget_; /// StackWidget中上次的Widget
  QTimer* heartbear_timer_; // 心跳包定时器
};

#endif // CHATDIALOG_H
