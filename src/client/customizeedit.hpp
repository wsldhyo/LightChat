#ifndef CUSTOMIZE_EDIT_HPP
#define CUSTOMIZE_EDIT_HPP
#include <QLineEdit>

/**
 * @brief 自定义编辑框，
 *  输入框默认不显示关闭按钮，当输入文字后显示关闭按钮，点击关闭按钮清空文字
 *  用于聊天界面侧边栏的搜索框
 *
 */
class CustomizeEdit : public QLineEdit {
  Q_OBJECT
public:
  CustomizeEdit(QWidget *parent = nullptr);
  void SetMaxLength(int maxLen);

protected:
  void focusOutEvent(QFocusEvent *event) override;

private:
  /**
   * @brief 限制输入框的最大输入长度
   *    如果输入文本超过长度限制，将截断并只显示前面部分的内容
   * @param text 要显示的文本 
   */
  void limitTextLength(QString text); 

  int _max_len;
signals:
  void sig_foucus_out();
};
#endif