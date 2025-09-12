#ifndef CHAT_VIEW_HPP
#define CHAT_VIEW_HPP
#include <QWidget>
class QVBoxLayout;
class QScrollArea;
/**
 * @brief 聊天界面ChatPage中的聊天对话框视窗。
 * 逐条显示历史消息，他人消息显示在最左侧，本人消息显示在最右侧
 * 目前采用ScrollArea + 垂直布局实现。
 * 也可以使用ListView实现，每个消息都是一个ListItem。
 *
 */

class ChatView : public QWidget
{
    Q_OBJECT
public:
    ChatView(QWidget *parent = Q_NULLPTR);
    void appendChatItem(QWidget *item);                 //尾插
    void prependChatItem(QWidget *item);                //头插
    void insertChatItem(QWidget *before, QWidget *item);//中间插
    void removeAllItem();
protected:
    bool eventFilter(QObject *o, QEvent *e) override;
    void paintEvent(QPaintEvent *event) override;
private slots:
    void onVScrollBarMoved(int min, int max);

private:
    void initStyleSheet();
private:
    //QWidget *m_pCenterWidget;
    QVBoxLayout *m_pVl;
    QScrollArea *m_pScrollArea;
    bool isAppended;

};

#endif