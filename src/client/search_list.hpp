#ifndef SEARCH_LIST_HPP
#define SEARCH_LIST_HPP

#include <QListWidget>

class LoadingDialog;
class SearchInfo;

/**
 * @brief ChatDialog搜索框的显示列表控件，显示搜索结果
 *
 */
class SearchList : public QListWidget {
  Q_OBJECT
public:
  SearchList(QWidget *parent = nullptr);
  void close_find_dlg();
  void set_search_edit(QWidget *edit);

protected:
  bool eventFilter(QObject *watched, QEvent *event) override;

private:
  void wait_pending(bool pending = true);
  void add_tip_item();
  void create_connection();
private slots:
  void slot_item_clicked(QListWidgetItem *item);
  void slot_user_search(std::shared_ptr<SearchInfo> si);

signals:
  void sig_switch_chat_item(std::shared_ptr<SearchInfo> si);

private:
  bool send_pending_;
  std::shared_ptr<QDialog> find_dlg_;
  QWidget *search_edit_;
  LoadingDialog *loadingDialog_;
};
#endif