#ifndef CHATUSERWID_HPP
#define CHATUSERWID_HPP

#include <memory>
#include <QWidget>
#include "../common/constant.hpp"
#include "struct_def.hpp"
#include "listitem_base.hpp"
namespace Ui {
class ChatUserWid;
}

class ChatUserWid : public ListItemBase
{
    Q_OBJECT

public:
    explicit ChatUserWid(QWidget *parent = nullptr);
    ~ChatUserWid();
    QSize sizeHint() const override;
    void SetInfo(std::shared_ptr<UserInfo> user_info);
    void SetInfo(std::shared_ptr<FriendInfo> friend_info);
    void ShowRedPoint(bool bshow);
    std::shared_ptr<UserInfo> GetUserInfo();
    void updateLastMsg(std::vector<std::shared_ptr<TextChatData>> msgs);
private:
    Ui::ChatUserWid *ui;
    std::shared_ptr<UserInfo> _user_info;
};

#endif // CHATUSERWID_H
