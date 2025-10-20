#ifndef CHAT_ITEM_BASE_HPP
#define CHAT_ITEM_BASE_HPP
#include "client_constant.hpp"
#include <QWidget>
class BubbleFrame;
class QLabel;
/**
 * @brief
 表示一条聊天消息的控件，以网格布局表示聊天气泡控件Bubble、用户头像控件等。
 * 大致布局如下
  他人消息布局
     None       Name                  Icon(头像）
     Spacer     Bubble(聊天气泡)       ICon(头像)

  本人发送的消息布局
     Icon(头像)    Name    None
     ICon(头像)   Bubble   Spacer

  聊天气泡Bubble是存储消息的主体控件，弹簧Spacer则用于让聊天气泡与边界存在一定距离
  Bubble可以根据消息长度压缩Spacer的空间
 *
 */
class ChatItemBase : public QWidget {
  Q_OBJECT
public:
  explicit ChatItemBase(ChatRole role, QWidget *parent = nullptr);
  void set_user_name(const QString &name);
  void set_user_icon(const QPixmap &icon);
  void set_widget(QWidget *w);
  int name_width() const;
  void set_spacer_width(int width);

private:
  void set_msg_state(SendMsgState msg_state);
  ChatRole role_;
  QLabel *name_label_;
  QLabel *icon_label_;
  QWidget *bubble_;
  QLabel* status_label_;
};

#endif