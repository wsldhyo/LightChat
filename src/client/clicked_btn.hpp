#ifndef CLICKEDBTN_HPP
#define CLICKEDBTN_HPP
#include <QPushButton>
// 自定义样式按钮，根据鼠标事件动态修改按钮状态，刷新样式表修改按钮样式（刷新为相应状态下的图片）
class ClickedBtn : public QPushButton {
  Q_OBJECT
public:
  ClickedBtn(QWidget *parent = nullptr);
  ~ClickedBtn();
  void set_state(QString nomal, QString hover, QString press);

protected:
  virtual void enterEvent(QEvent *event) override;             // 鼠标进入
  virtual void leaveEvent(QEvent *event) override;             // 鼠标离开
  virtual void mousePressEvent(QMouseEvent *event) override;   // 鼠标按下
  virtual void mouseReleaseEvent(QMouseEvent *event) override; // 鼠标释放
private:
  QString normal_;
  QString hover_;
  QString press_;
};

#endif