#ifndef CHAT_PAGE_HPP
#define CHAT_PAGE_HPP
#include <QWidget>
struct UserInfo;
struct TextChatData;
namespace Ui {
class ChatPage;
}
/**
 * @brief 聊天界面类，主要包含聊天消息视窗ChatView和消息编辑框MessageTestEdit
 *
 */
class ChatPage : public QWidget {
  Q_OBJECT

public:
  explicit ChatPage(QWidget *parent = nullptr);
  ~ChatPage();
  void set_user_info(std::shared_ptr<UserInfo> user_info);
  void append_chat_msg(std::shared_ptr<TextChatData> msg);

signals:
  void sig_append_send_chat_msg(std::shared_ptr<TextChatData>);

protected:
  void paintEvent(QPaintEvent *event) override;

private slots:
  void on_send_btn_clicked();

private:
  Ui::ChatPage *ui;
  std::shared_ptr<UserInfo> user_info_;
};
#endif