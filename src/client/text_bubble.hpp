#ifndef TEXT_BUBBLE_HPP
#define TEXT_BUBBLE_HPP
#include "bubble_frame.hpp"
class QTextEdit;
/**
 * @brief 纯文本气聊天泡框
 *
 */
class TextBubble : public BubbleFrame {
  Q_OBJECT
public:
  TextBubble(ChatRole role, const QString &text, QWidget *parent = nullptr);
protected:
  //void resizeEvent(QResizeEvent *event) override;
  //bool eventFilter(QObject *o, QEvent *e) override;

private:
  void adjust_text_height();
  void set_plain_text(const QString &text);
  void init_style_sheet();

private:
  QTextEdit *text_edit_;
};
#endif