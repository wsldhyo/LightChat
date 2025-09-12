#ifndef BUBBLE_H
#define BUBBLE_H

#include <QFrame>
#include "client_constant.hpp"
class QHBoxLayout;
class BubbleFrame : public QFrame
{
    Q_OBJECT
public:
    BubbleFrame(ChatRole role, QWidget *parent = nullptr);
    void setMargin(int margin);
    //inline int margin(){return margin;}
    void setWidget(QWidget *w);
protected:
    void paintEvent(QPaintEvent *e);
    ChatRole m_role;
private:
    QHBoxLayout *m_pHLayout;
     int      m_margin;
};

#endif // BUBBLE_H
