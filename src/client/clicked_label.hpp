#ifndef CLICKEDLABEL_HPP
#define CLICKEDLABEL_HPP
#include "client_constant.hpp"
#include <QLabel>
class ClickedLabel : public QLabel {
  Q_OBJECT
public:
  ClickedLabel(QWidget *parent);
  virtual void mousePressEvent(QMouseEvent *ev) override;
  virtual void enterEvent(QEvent *event) override;
  virtual void leaveEvent(QEvent *event) override;
  virtual void mouseReleaseEvent(QMouseEvent *event) override;

  void set_state(QString normal = "", QString hover = "", QString press = "",
                QString select = "", QString select_hover = "",
                QString select_press = "");

  ClickLbState get_cur_state();
  bool set_cur_state(ClickLbState state);
  void reset_normal_state();

protected:
private:
  QString normal_;
  QString normal_hover_;
  QString normal_press_;

  QString selected_;
  QString selected_hover_;
  QString selected_press_;

  ClickLbState curstate_;
signals:
  void clicked(QString, ClickLbState);
};

#endif // CLICKEDLABEL_H