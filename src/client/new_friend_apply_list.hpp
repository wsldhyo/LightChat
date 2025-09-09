#ifndef NEW_FRIEND_APPLY_LIST_HPP
#define NEW_FRIEND_APPLY_LIST_HPP
#include <QListWidget>

/**
 * @brief 好友请求列表控件（继承 QListWidget）
 *        用于显示好友申请项，并处理鼠标事件。
 */
class NewFriendApplyList : public QListWidget {
  Q_OBJECT
public:
  /**
   * @brief 构造函数
   * @param parent 父窗口
   */
  NewFriendApplyList(QWidget *parent = nullptr);

protected:
  /**
   * @brief 事件过滤器，用于控制滚动条显示与隐藏
   * @param watched 被观察的对象
   * @param event 事件对象
   * @return 是否拦截事件
   */
  bool eventFilter(QObject *watched, QEvent *event) override;

signals:
  /**
   * @brief 通知外部是否显示搜索框
   * @param show 是否显示
   */
  void sig_show_search(bool);
};

#endif // NEW_FRIEND_APPLY_LIST_HPP