#ifndef STATE_WIDGET_HPP
#define STATE_WIDGET_HPP
#include "client_constant.hpp"
#include <QWidget>
class QLabel;

/**
 * @brief ChatDialog侧边栏的控件，用于切换不同界面：聊天会话界面或联系人界面 
 * 
 */
class StateWidget : public QWidget {
  Q_OBJECT
public:
  explicit StateWidget(QWidget *parent = nullptr);

  void set_state(QString normal = "", QString hover = "", QString press = "",
                QString select = "", QString select_hover = "",
                QString select_press = "");

  ClickLbState get_cur_state();
  void clear_state();

  void set_selected(bool bselected);
  void add_red_point();
  void show_red_point(bool show = true);

protected:
  void paintEvent(QPaintEvent *event) override;

  virtual void mousePressEvent(QMouseEvent *ev) override;
  virtual void mouseReleaseEvent(QMouseEvent *ev) override;
  virtual void enterEvent(QEvent *event) override;
  virtual void leaveEvent(QEvent *event) override;

private:
  QString normal_;
  QString normal_hover_;
  QString normal_press_;

  QString selected_;
  QString selected_hover_;
  QString selected_press_;

  ClickLbState curstate_;
  QLabel *red_point_;

signals:
  void clicked(void);

signals:

public slots:
};
#endif