#ifndef CHATITEMBASE_H
#define CHATITEMBASE_H

#include "client_constant.hpp"
#include <QWidget>
class QLabel;
class BubbleFrame;

class ChatItemBase : public QWidget {
  Q_OBJECT
public:
  explicit ChatItemBase(ChatRole role, QWidget *parent = nullptr);
  void setUserName(const QString &name);
  void setUserIcon(const QPixmap &icon);
  void setWidget(QWidget *w);
  int name_width()const noexcept;

private:
  ChatRole m_role;
  QLabel *m_pNameLabel;
  QLabel *m_pIconLabel;
  QWidget *m_pBubble;
};

#endif // CHATITEMBASE_H
