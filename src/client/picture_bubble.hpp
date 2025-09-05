#ifndef PICTURE_BUBBLE_HPP
#define PICTURE_BUBBLE_HPP
#include "bubble_frame.hpp"
/**
 * @brief 图片聊天气泡框，消息内容为图片
 * 
 */
class PictureBubble : public BubbleFrame
{
    Q_OBJECT
public:
    PictureBubble(const QPixmap &picture, ChatRole role, QWidget *parent = nullptr);
};
#endif