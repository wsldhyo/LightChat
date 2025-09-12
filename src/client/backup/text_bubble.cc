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
#include <QTimer>
#include <QtGlobal>
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

  setPlainText(text);
  setWidget(text_edit_);
  initStyleSheet();

  // 文本文档排版完成后，自动调整气泡高度
  connect(text_edit_->document()->documentLayout(),
          &QAbstractTextDocumentLayout::documentSizeChanged, this,
          [this](const QSizeF &newSize) {
            qreal doc_margin = text_edit_->document()->documentMargin();
            int vMargin = this->layout()->contentsMargins().top();
            int newHeight = newSize.height() + doc_margin * 2 + vMargin * 2;
            setFixedHeight(newHeight);
            qDebug() << "documentSizeChanged:" << newSize
                     << "bubble height:" << newHeight;
          });

  text_edit_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
}

// 设置文本（只负责宽度，不再强制算高度）
void TextBubble::setPlainText(const QString &text) {
    text_edit_->setPlainText(text);

    // 原有的宽度计算逻辑可以保留用于估算，但避免设置严格的maximumWidth
    qreal doc_margin = text_edit_->document()->documentMargin();
    int margin_left = this->layout()->contentsMargins().left();
    int margin_right = this->layout()->contentsMargins().right();
    QFontMetricsF fm(text_edit_->font());
    QTextDocument *doc = text_edit_->document();

    int max_width = 0;
    for (QTextBlock it = doc->begin(); it != doc->end(); it = it.next()) {
        int txtW = int(fm.horizontalAdvance(it.text()));
        max_width = std::max(max_width, txtW);
    }

    // 关键修改：不再设置严格的maximumWidth，而是使用一个较大的值或注释掉
    // 或者设置为父容器宽度的某个比例，但这需要获取父容器宽度，逻辑更复杂
    // setMaximumWidth(max_width + doc_margin * 2 + margin_left + margin_right + 20);
    
    // 替代方案：可以设置一个建议的最小宽度，但最大宽度允许扩展
    setMinimumWidth(100); // 设置一个最小宽度
    // 最大宽度可以设置为一个很大的值，或者不设置（默认为16777215）
    // setMaximumWidth(16777215); // 如果需要显式设置的话

    qDebug() << "Text bubble min width:" << minimumWidth() << ", max width:" << maximumWidth();
}
// 样式
void TextBubble::initStyleSheet() {
  text_edit_->setStyleSheet(
      "QTextEdit { background: transparent; border: none }");
}