#ifndef CUSTOM_LINEEDIT_HPP
#define CUSTOM_LINEEDIT_HPP
#include <QLineEdit>

class CustomizeEdit : public QLineEdit {
  Q_OBJECT
public:
  CustomizeEdit(QWidget * _parent = nullptr);
  void set_max_length(int _max_len);

protected:
  void focusOutEvent(QFocusEvent * _event) override;
private:
signals:
    // 失去焦点
    void sig_onblur();

private:
    // 限制编辑框显示的文本长度
    void limit_text_length(QString _text) ;
    void create_connection();
    int max_len_;
};
#endif