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
  void setFrameMiniWidth(int width);
protected:
  //void resizeEvent(QResizeEvent *event) override;
  //bool eventFilter(QObject *o, QEvent *e) override;

private:
  void adjustTextHeight();
  void setPlainText(const QString &text);
  void initStyleSheet();

private:
  QTextEdit *text_edit_;
};
#endif