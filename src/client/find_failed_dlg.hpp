#ifndef FIND_FAILED_DLG_HPP
#define FIND_FAILED_DLG_HPP

#include <QDialog>

namespace Ui {
class FindFailedDlg;
}
/**
 * @brief 查找失败对话框 
 * 
 */
class FindFailedDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FindFailedDlg(QWidget *parent = nullptr);
    ~FindFailedDlg();

private slots:


    void on_fail_sure_btn_clicked();

private:
    Ui::FindFailedDlg *ui;
};
#endif // FIND_FAILED_DLG_HPP