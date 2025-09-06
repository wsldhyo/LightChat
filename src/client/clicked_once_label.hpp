#ifndef CLICKED_ONCE_LABEL
#define CLICKED_ONCE_LABEL
#include <QLabel>
/**
 * @brief 支持点击一次的标签
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