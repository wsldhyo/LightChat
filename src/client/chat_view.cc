#include "chat_view.hpp"
#include <QApplication>
#include <QDebug>
#include <QEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QStyleOption>
#include <QTimer>
#include <QVBoxLayout>
ChatView::ChatView(QWidget *parent) : QWidget(parent), is_appended_(false) {
  QVBoxLayout *pMainLayout = new QVBoxLayout();
  this->setLayout(pMainLayout);
  pMainLayout->setMargin(0);

  scroll_area_ = new QScrollArea();
  scroll_area_->setObjectName("chat_area");
  pMainLayout->addWidget(scroll_area_);

  QWidget *w = new QWidget(this);
  w->setObjectName("chat_bg");
  w->setAutoFillBackground(true);
  QVBoxLayout *pHLayout_1 = new QVBoxLayout();
  pHLayout_1->addWidget(new QWidget(), 100000);
  w->setLayout(pHLayout_1);
  scroll_area_->setWidget(w); //应该时在QSCrollArea构造后执行 才对

  scroll_area_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  QScrollBar *pVScrollBar = scroll_area_->verticalScrollBar();
  connect(pVScrollBar, &QScrollBar::rangeChanged, this,
          &ChatView::on_vscroll_bar_moved);
  //把垂直ScrollBar放到上边 而不是原来的并排
  QHBoxLayout *pHLayout_2 = new QHBoxLayout();
  pHLayout_2->addWidget(pVScrollBar, 0, Qt::AlignRight);
  pHLayout_2->setMargin(0);
  scroll_area_->setLayout(pHLayout_2);
  pVScrollBar->setHidden(true);

  scroll_area_->setWidgetResizable(true);
  scroll_area_->installEventFilter(this);
  init_style_sheet();
}

void ChatView::append_chat_item(QWidget *item) {
  QVBoxLayout *vl =
      qobject_cast<QVBoxLayout *>(scroll_area_->widget()->layout());
  qDebug() << "vl->count() is " << vl->count();
  vl->insertWidget(vl->count() - 1, item);
  is_appended_ = true;
}

void ChatView::prepend_chat_item(QWidget *item) {}

void ChatView::insert_chat_item(QWidget *before, QWidget *item) {}

void ChatView::remove_all_item() {
  QVBoxLayout *layout =
      qobject_cast<QVBoxLayout *>(scroll_area_->widget()->layout());

  int count = layout->count();

  for (int i = 0; i < count - 1; ++i) {
    QLayoutItem *item = layout->takeAt(0); // 始终从第一个控件开始删除
    if (item) {
      if (QWidget *widget = item->widget()) {
        delete widget;
      }
      delete item;
    }
  }
}

bool ChatView::eventFilter(QObject *o, QEvent *e) {
  /*if(e->type() == QEvent::Resize && o == )
  {

  }
  else */
  if (e->type() == QEvent::Enter && o == scroll_area_) {
    scroll_area_->verticalScrollBar()->setHidden(
        scroll_area_->verticalScrollBar()->maximum() == 0);
  } else if (e->type() == QEvent::Leave && o == scroll_area_) {
    scroll_area_->verticalScrollBar()->setHidden(true);
  }
  return QWidget::eventFilter(o, e);
}

void ChatView::paintEvent(QPaintEvent *event) {
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

void ChatView::on_vscroll_bar_moved(int min, int max) {
  if (is_appended_) //添加item可能调用多次
  {
    QScrollBar *pVScrollBar = scroll_area_->verticalScrollBar();
    pVScrollBar->setSliderPosition(pVScrollBar->maximum());
    // 500毫秒内可能调用多次
    QTimer::singleShot(500, [this]() { is_appended_ = false; });
  }
}

void ChatView::init_style_sheet() {
  //    QScrollBar *scrollBar = scroll_area_->verticalScrollBar();
  //    scrollBar->setStyleSheet("QScrollBar{background:transparent;}"
  //                             "QScrollBar:vertical{background:transparent;width:8px;}"
  //                             "QScrollBar::handle:vertical{background:red;
  //                             border-radius:4px;min-height:20px;}"
  //                             "QScrollBar::add-line:vertical{height:0px}"
  //                             "QScrollBar::sub-line:vertical{height:0px}"
  //                             "QScrollBar::add-page:vertical
  //                             {background:transparent;}"
  //                             "QScrollBar::sub-page:vertical
  //                             {background:transparent;}");
}
