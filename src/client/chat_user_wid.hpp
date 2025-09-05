#ifndef CHAT_USER_WID_HPP
#define CHAT_USER_WID_HPP
#include "chat_user_list.hpp"
#include "list_item_base.hpp"
namespace Ui {
class ChatUserWid;
}
/**
 * @brief 自定义聊天会话列表的列表项，依次显示头像，用户名、最近消息以及最近消息的时间
 * 
 */
class ChatUserWid : public ListItemBase
{
    Q_OBJECT

public:
    explicit ChatUserWid(QWidget *parent = nullptr);
    ~ChatUserWid();

    QSize sizeHint() const override {
        return QSize(250, 70); // 返回自定义的尺寸
    }

    void SetInfo(QString name, QString head, QString msg);

private:
    Ui::ChatUserWid *ui;
    QString name_;
    QString head_;
    QString msg_;
};
#endif