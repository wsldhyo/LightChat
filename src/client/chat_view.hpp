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
    void append_chat_item(QWidget *item);                 //尾插
    void prepend_chat_item(QWidget *item);                //头插
    void insert_chat_item(QWidget *before, QWidget *item);//中间插
    void remove_all_item();
protected:
    bool eventFilter(QObject *o, QEvent *e) override;
    void paintEvent(QPaintEvent *event) override;
private slots:
    void on_vscroll_bar_moved(int min, int max);

private:
    void init_style_sheet();
private:
    //QWidget *m_pCenterWidget;
    QVBoxLayout *v_layout_;
    QScrollArea *scroll_area_;
    bool is_appended_;

};

#endif