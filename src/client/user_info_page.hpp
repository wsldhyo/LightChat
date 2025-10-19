#ifndef USER_INFO_PAGE_HPP
#define USER_INFO_PAGE_HPP
#include <QWidget>

namespace Ui {
class UserInfoPage;
}

class UserInfoPage : public QWidget
{
    Q_OBJECT

public:
    explicit UserInfoPage(QWidget *parent = nullptr);
    ~UserInfoPage();

private slots:
    void on_up_btn_clicked();

private:
    Ui::UserInfoPage *ui;
};
#endif // USER_INFO_PAGE_HPP