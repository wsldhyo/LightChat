#ifndef CLICKED_ONCE_LABEL
#define CLICKED_ONCE_LABEL
#include <QLabel>
/**
 * @brief 支持点击一次的标签, 当鼠标左键释放时，发射clicked信号,
 * 类似QPushButton，可在ApplyFriendDialog的好友标签列表中用作
 * more控件，点击后展开更多好友标签
 * 
 */
class ClickedOnceLabel : public QLabel
{
    Q_OBJECT
public:
    ClickedOnceLabel(QWidget *parent=nullptr);
    virtual void mouseReleaseEvent(QMouseEvent *ev) override;
signals:
    void clicked(QString);
};
#endif // CLICKED_ONCE_LABEL