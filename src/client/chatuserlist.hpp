#ifndef CHAT_USER_LSIT_HPP
#define CHAT_USER_LSIT_HPP
#include <QListWidget>
/**
 * @brief 聊天对话列表类，显示对方头像，支持滚动选择聊天对话
 * 
 */
class ChatUserList : public QListWidget {
  Q_OBJECT
public:
  ChatUserList(QWidget *parent = nullptr);

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

signals:
  void sig_loading_chat_user();
};

#endif