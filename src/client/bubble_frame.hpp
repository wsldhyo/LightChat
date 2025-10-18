#ifndef BUBBLE_FRAME_HPP
#define BUBBLE_FRAME_HPP

#include "client_constant.hpp"
#include <QFrame>
class QHBoxLayout;

/**
 * @brief 聊天气泡的基类，基于此类可以实现各类聊天气泡，如纯文本聊天气泡TextBubble、
 * 图片聊天气泡PictureBubble等
 * 
 */
class BubbleFrame : public QFrame
{
    Q_OBJECT
public:
    BubbleFrame(ChatRole role, QWidget *parent = nullptr);
    void set_margin(int margin);
    //inline int margin(){return margin;}
    void set_widget(QWidget *w);
protected:
    void paintEvent(QPaintEvent *e) override;
private:
    QHBoxLayout *h_layout_;
    ChatRole role_;
     int      margin_;
};

#endif