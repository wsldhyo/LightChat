#ifndef FRIEND_INFO_PAGE_HPP
#define FRIEND_INFO_PAGE_HPP

#include <QWidget>
struct UserInfo;
namespace Ui {
class FriendInfoPage;
}

class FriendInfoPage : public QWidget {
  Q_OBJECT

public:
  explicit FriendInfoPage(QWidget *parent = nullptr);
  ~FriendInfoPage();
  void SetInfo(std::shared_ptr<UserInfo> ui);
private slots:
  void on_msg_chat_clicked();

private:
  Ui::FriendInfoPage *ui;
  std::shared_ptr<UserInfo> user_info_;
signals:
  void sig_jump_chat_item(std::shared_ptr<UserInfo> si);
};

#endif // FRIEND_INFO_PAGE_HPP
