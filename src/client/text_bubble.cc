#include "text_bubble.hpp"
#include <QDebug>
#include <QFont>
#include <QFontMetricsF>
#include <QHBoxLayout>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextLayout>
#include <QTimer>
TextBubble::TextBubble(ChatRole role, const QString &text, QWidget *parent)
    : BubbleFrame(role, parent) {
  text_edit_ = new QTextEdit();
  text_edit_->setReadOnly(true);
  text_edit_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  text_edit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  text_edit_->installEventFilter(this);
  QFont font("Microsoft YaHei");
  font.setPointSize(12);
  text_edit_->setFont(font);
  setPlainText(text);
  setWidget(text_edit_);
  initStyleSheet();
}

bool TextBubble::eventFilter(QObject *o, QEvent *e) {
  if (text_edit_ == o && e->type() == QEvent::Paint) {
    adjustTextHeight(); // PaintEvent中设置
  }
  return BubbleFrame::eventFilter(o, e);
}

void TextBubble::setPlainText(const QString &text) {
  text_edit_->setPlainText(text);
  // text_edit_->setHtml(text);
  //找到段落中最大宽度
  qreal doc_margin = text_edit_->document()->documentMargin();
  int margin_left = this->layout()->contentsMargins().left();
  int margin_right = this->layout()->contentsMargins().right();
  QFontMetricsF fm(text_edit_->font());
  QTextDocument *doc = text_edit_->document();
  int max_width = 0;
  //遍历每一段找到 最宽的那一段
  for (QTextBlock it = doc->begin(); it != doc->end();
       it = it.next()) //字体总长
  {
    int txtW = int(fm.width(it.text()));
    max_width = max_width < txtW ? txtW : max_width; //找到最长的那段
  }
  //设置这个气泡的最大宽度 只需要设置一次
  setMaximumWidth(max_width + doc_margin * 2 +
                  (margin_left + margin_right)); //设置最大宽度
}

void TextBubble::adjustTextHeight() {
  qreal doc_margin =
      text_edit_->document()->documentMargin(); //字体到边框的距离默认为4
  QTextDocument *doc = text_edit_->document();
  qreal text_height = 0;
  //把每一段的高度相加=文本高
  for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next()) {
    QTextLayout *pLayout = it.layout();
    QRectF text_rect = pLayout->boundingRect(); //这段的rect
    text_height += text_rect.height();
  }
  int vMargin = this->layout()->contentsMargins().top();
  //设置这个气泡需要的高度 文本高+文本边距+TextEdit边框到气泡边框的距离
  qDebug() << "adjust text height:" << text_height + doc_margin * 2 + vMargin * 2;
  setFixedHeight(text_height + doc_margin * 2 + vMargin * 2);
}

void TextBubble::initStyleSheet() {
  text_edit_->setStyleSheet("QTextEdit{background:transparent;border:none}");
}