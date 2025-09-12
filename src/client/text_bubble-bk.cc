#include "text_bubble.hpp"
#include <QDebug>
#include <QFont>
#include <QFontMetricsF>
#include <QLayout>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextLayout>
#include <QTimer>

TextBubble::TextBubble(ChatRole role, const QString &text, QWidget *parent)
    : BubbleFrame(role, parent) {
  m_pTextEdit = new QTextEdit();
  m_pTextEdit->setReadOnly(true);
  m_pTextEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_pTextEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_pTextEdit->installEventFilter(this);
  QFont font("Microsoft YaHei");
  font.setPointSize(12);
  m_pTextEdit->setFont(font);
  setPlainText(text);
  setWidget(m_pTextEdit);
  initStyleSheet();
}

bool TextBubble::eventFilter(QObject *o, QEvent *e) {
  return BubbleFrame::eventFilter(o, e);
}

void TextBubble::resizeEvent(QResizeEvent *event) {
  QFrame::resizeEvent(event); // 保持基类处理
  adjustTextHeight();
}

void TextBubble::setPlainText(const QString &text) {
  m_pTextEdit->setPlainText(text);

  qreal doc_margin = m_pTextEdit->document()->documentMargin();
  int margin_left = this->layout()->contentsMargins().left();
  int margin_right = this->layout()->contentsMargins().right();

  QFontMetrics fm(m_pTextEdit->font());
  int txtW = fm.horizontalAdvance(text);

  // QTextEdit 边框宽度
  int frame_w = m_pTextEdit->frameWidth();

  // 额外留 2~4 像素，避免因字体度量误差导致换行
  int extra = 4;

  int bubble_width = txtW + int(doc_margin * 2) + margin_left + margin_right +
                     frame_w * 2 + extra;

  setMaximumWidth(bubble_width);
}

void TextBubble::adjustTextHeight() {

  QTextDocument *doc = m_pTextEdit->document();
  doc->setTextWidth(
      m_pTextEdit->viewport()->width()); // 根据 TextEdit 当前宽度换行

  int vMargin = this->layout()->contentsMargins().top();
  qreal doc_margin = doc->documentMargin();

  int new_height = int(doc->size().height()) + doc_margin * 2 + vMargin * 2 +
                   m_pTextEdit->frameWidth() * 2;

  setFixedHeight(new_height);
  qDebug() << "adjustHeight:" << new_height;
}

void TextBubble::initStyleSheet() {
  m_pTextEdit->setStyleSheet("QTextEdit{background:transparent;border:none}");
}
