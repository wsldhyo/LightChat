#include "text_bubble.hpp"
#include <QDebug>
#include <QFont>
#include <QFontMetricsF>
#include <QHBoxLayout>
#include <QPlainTextDocumentLayout>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextEdit>
#include <QTextLayout>

// 构造函数
TextBubble::TextBubble(ChatRole role, const QString &text, QWidget *parent)
    : BubbleFrame(role, parent) {
  text_edit_ = new QTextEdit();
  text_edit_->setReadOnly(true);
  text_edit_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  text_edit_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  QFont font("Microsoft YaHei");
  font.setPointSize(12);
  text_edit_->setFont(font);
  this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  setPlainText(text);
  setWidget(text_edit_);
  initStyleSheet();

  // 文本文档排版完成后，自动调整气泡高度
  connect(text_edit_->document()->documentLayout(),
          &QAbstractTextDocumentLayout::documentSizeChanged, this,
          [this](const QSizeF &newSize) {
            qreal doc_margin = text_edit_->document()->documentMargin();
            int vMargin = this->layout()->contentsMargins().top();
            setFixedHeight(newSize.height() + doc_margin * 2 + vMargin * 2);
            // qDebug() << "adjust text height:"
            // << newSize.height() + doc_margin * 2 + vMargin * 2;
            // 强制父布局立即刷新
            if (auto lay = this->parentWidget() ? this->parentWidget()->layout()
                                                : nullptr) {
              lay->invalidate();
              lay->activate();
            }
            this->updateGeometry();
          });
}

// 设置文本（只负责宽度，不再强制算高度）
void TextBubble::setPlainText(const QString &text) {
  text_edit_->setPlainText(text);

  // 计算文本的理想宽度
  qreal doc_margin = text_edit_->document()->documentMargin();
  int margin_left = this->layout()->contentsMargins().left();
  int margin_right = this->layout()->contentsMargins().right();

  QFontMetrics fm(text_edit_->font());
  int txtW = fm.horizontalAdvance(text);

  // QTextEdit 边框宽度
  int frame_w = text_edit_->frameWidth();

  // 额外留 2~4 像素，避免因字体度量误差导致换行
  int extra = 4;

  int bubble_width = txtW + int(doc_margin * 2) + margin_left + margin_right +
                     frame_w * 2 + extra;

  setMaximumWidth(bubble_width);
  // qDebug() << "bubble max width" << bubble_width;

}

// 样式
void TextBubble::initStyleSheet() {
  text_edit_->setStyleSheet(
      "QTextEdit { background: transparent; border: none }");
}