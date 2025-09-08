#ifndef FRIEND_LABEL_HPP
#define FRIEND_LABEL_HPP
#include <QFrame>
#include <QString>

namespace Ui {
class FriendLabel;
}

/**
 * @brief ApplyFriendDialog中，用于表示好友标签的控件类
 * 
 */
class FriendLabel : public QFrame {
  Q_OBJECT

public:
  explicit FriendLabel(QWidget *parent = nullptr);
  ~FriendLabel();
  void set_text(QString text);
  int width();
  int height();
  QString text();
public slots:
  void slot_close();
signals:
  void sig_close(QString);

private:
  Ui::FriendLabel *ui;
  QString text_;
  int width_;
  int height_;
};

#endif // FRIEND_LABEL_HPP