#ifndef CHAT_PAGE_HPP
#define CHAT_PAGE_HPP
#include <QWidget>
namespace Ui {
class ChatPage;
}
/**
 * @brief 聊天界面类，主要包含消息对话框和消息编辑框
 *
 */
class ChatPage : public QWidget {
  Q_OBJECT

public:
  explicit ChatPage(QWidget *parent = nullptr);
  ~ChatPage();
protected:
    void paintEvent(QPaintEvent *event)override;

private:
  Ui::ChatPage *ui;
};
#endif