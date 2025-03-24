#ifndef PWD_VISIBLE_LBL
#define PWD_VISIBLE_LBL
#include "../common/constant.hpp"
#include <QLabel>
class PwdVisibleLbl : public QLabel {
  Q_OBJECT
public:
  PwdVisibleLbl(QWidget *_parent = nullptr);
  // 处理鼠标按压逻辑
  void mousePressEvent(QMouseEvent *_event) override;
  // 处理鼠标悬停进入的逻辑
  void enterEvent(QEvent *_event) override;
  // 处理鼠标悬停离开的逻辑
  void leaveEvent(QEvent *_event) override;
  void set_state(QString _normal = "", QString _hover = "", QString _press = "",
                 QString _select = "", QString _select_hover = "",
                 QString _select_press = "");
  PwdVisibleState get_state();

protected:
private:
  QString normal_;
  QString normal_hover_;
  QString normal_press_;
  QString selected_;
  QString selected_hover_;
  QString selected_press_;
  PwdVisibleState curstate_;
signals:
  void clicked(void);
};
#endif