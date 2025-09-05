#ifndef FIND_SUCCESS_DIALOG_HPP
#define FIND_SUCCESS_DIALOG_HPP

#include "user_data.hpp"
#include <QDialog>
#include <memory>
namespace Ui {
class FindSuccessDlg;
}

class FindSuccessDlg : public QDialog {
  Q_OBJECT

public:
  explicit FindSuccessDlg(QWidget *parent = nullptr);
  ~FindSuccessDlg();
  void set_search_info(std::shared_ptr<SearchInfo> si);
private slots:
  void on_add_friend_btn_clicked();

private:
  Ui::FindSuccessDlg *ui;
  QWidget *parent_;
  std::shared_ptr<SearchInfo> si_;
};
#endif // FIND_SUCCESS_DIALOG_HPP
