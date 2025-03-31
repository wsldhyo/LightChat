#ifndef CHAT_USER_LIST_HPP
#define CHAT_USER_LIST_HPP
#include <QListWidget>

class ChatUserList : public QListWidget
{
    Q_OBJECT
public:
    ChatUserList(QWidget* _parent = nullptr);
protected:
    bool eventFilter(QObject* _watched, QEvent* _event)override;

private:
signals:
    void sig_loading_chat_user();    
private:

};
#endif