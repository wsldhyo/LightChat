#ifndef TEXTBUBBLE_H
#define TEXTBUBBLE_H

#include "bubble_frame.hpp"
class QTextEdit;
class TextBubble : public BubbleFrame {
  Q_OBJECT
public:
  TextBubble(ChatRole role, const QString &text, QWidget *parent = nullptr);

protected:
  bool eventFilter(QObject *o, QEvent *e) override;
  void resizeEvent(QResizeEvent *event) override;

private:
  void adjustTextHeight();
  void setPlainText(const QString &text);
  void initStyleSheet();

private:
  QTextEdit *m_pTextEdit;
};

#endif // TEXTBUBBLE_H
