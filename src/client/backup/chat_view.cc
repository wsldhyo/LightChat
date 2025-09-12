#include "chat_view.hpp"
#include <QApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QStyleOption>
#include <QTimer>
#include <QVBoxLayout>
ChatView::ChatView(QWidget *parent) : QWidget(parent), is_appended_(false) {
  // ChatView 的主垂直布局（整体容器）
  QVBoxLayout *pMainLayout = new QVBoxLayout();
  this->setLayout(pMainLayout);
  pMainLayout->setMargin(0);

  // 聊天消息的滚动区域
  scrollarea_ = new QScrollArea();
  scrollarea_->setObjectName("chat_area");
  pMainLayout->addWidget(scrollarea_);

  // 滚动区域中的承载窗口（所有消息都会加到这个窗口里）
  QWidget *w = new QWidget(this);
  w->setObjectName("chat_bg");
  w->setAutoFillBackground(true); // 允许用样式表绘制背景

  // w 的垂直布局（用于存放消息项）
  QVBoxLayout *pVLayout_1 = new QVBoxLayout();

  // 在最底部放一个“占位部件”，拉伸因子设为很大
  // 保证消息始终从上往下排列，底部有空白，不会挤在中间
  pVLayout_1->addWidget(new QWidget(), 100000);

  w->setLayout(pVLayout_1);
  scrollarea_->setWidget(w); // 把 w 放到滚动区域里

  // 默认不显示滚动条（需要时才显示）
  scrollarea_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  QScrollBar *pVScrollBar = scrollarea_->verticalScrollBar();

  // 当内容范围变化时（新消息加入），触发槽函数自动滚动到底部
  connect(pVScrollBar, &QScrollBar::rangeChanged, this,
          &ChatView::onVScrollBarMoved);

  // 自定义滚动条布局：把滚动条单独放到右边，而不是默认叠加在 QScrollArea 上
  QHBoxLayout *pHLayout_2 = new QHBoxLayout();
  pHLayout_2->addWidget(pVScrollBar, 0, Qt::AlignRight);
  pHLayout_2->setMargin(0);
  scrollarea_->setLayout(pHLayout_2);
  pVScrollBar->setHidden(true); // 初始隐藏

  // 开启自动调整大小（消息区域会随内容大小变化）
  scrollarea_->setWidgetResizable(true);

  // 安装事件过滤器，用于处理“鼠标进入/离开时显示或隐藏滚动条”
  scrollarea_->installEventFilter(this);

  initStyleSheet(); // 初始化样式（颜色、背景等）
}

// 插入一条聊天消息
void ChatView::appendChatItem(QWidget *item) {
  // 找到 scrollarea 内部 widget 的布局（QVBoxLayout）
  QVBoxLayout *vl =
      qobject_cast<QVBoxLayout *>(scrollarea_->widget()->layout());

  // 把消息插入到“占位部件”之前（所以消息在上面，底部永远留空）
  vl->insertWidget(vl->count() - 1, item, 0, Qt::AlignRight | Qt::AlignTop);
  is_appended_ = true; // 标记：刚刚插入了新消息
}

// 事件过滤器：控制滚动条显示/隐藏
bool ChatView::eventFilter(QObject *o, QEvent *e) {
  if (e->type() == QEvent::Enter && o == scrollarea_) {
    // 鼠标进入时：如果内容超出一屏，才显示滚动条
    scrollarea_->verticalScrollBar()->setHidden(
        scrollarea_->verticalScrollBar()->maximum() == 0);
  } else if (e->type() == QEvent::Leave && o == scrollarea_) {
    // 鼠标离开时：始终隐藏滚动条
    scrollarea_->verticalScrollBar()->setHidden(true);
  }
  return QWidget::eventFilter(o, e);
}

// 重绘事件：确保样式表的背景能正常绘制（QWidget 默认不绘制背景）
void ChatView::paintEvent(QPaintEvent *event) {
  QStyleOption opt;
  opt.init(this);
  QPainter p(this);
  style()->drawPrimitive(QStyle::PE_Widget, &opt, &p, this);
}

// 每次有新消息加入时，滚动条会自动滚动到底部
void ChatView::onVScrollBarMoved(int min, int max) {
  if (is_appended_) {
    QScrollBar *pVScrollBar = scrollarea_->verticalScrollBar();
    // 滚动到最底部
    pVScrollBar->setSliderPosition(pVScrollBar->maximum());
    // 延时 500ms 后清除标记，避免连续插入多条消息时滚动逻辑抖动
    QTimer::singleShot(500, [this]() { is_appended_ = false; });
  }
}

void ChatView::initStyleSheet() {
  // TODO: 设置样式表，如背景色、聊天气泡样式等
}
