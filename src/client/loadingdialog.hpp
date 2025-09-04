#ifndef LOADING_DIALOG_HPP
#define LOADING_DIALOG_HPP
#include <QDialog>

namespace Ui {
class LoadingDialog;
}
/**
 * @brief 显示加载时的Gif
 * 聊天会话列表、联系人列表向下滚动时，如果还有需要加载的项，则在加载时显示加载的Gif
 *
 */
class LoadingDialog : public QDialog {
  Q_OBJECT

public:
  explicit LoadingDialog(QWidget *parent = nullptr);
  ~LoadingDialog();

private:
  Ui::LoadingDialog *ui;
};
#endif