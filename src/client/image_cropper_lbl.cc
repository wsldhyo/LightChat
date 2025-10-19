#include "image_cropper_lbl.hpp"
#include <QBitmap>
#include <QDebug>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

ImageCropperLabel::ImageCropperLabel(int width, int height, QWidget *parent)
    : QLabel(parent) {
  this->setFixedSize(width, height);
  this->setAlignment(Qt::AlignCenter);
  this->setMouseTracking(true);

  borderPen.setWidth(1);
  borderPen.setColor(Qt::white);
  borderPen.setDashPattern(QVector<qreal>() << 3 << 3 << 3 << 3);
}

void ImageCropperLabel::setOriginalImage(const QPixmap &pixmap) {
  originalImage = pixmap; // 保存原始图片

  // 获取原图尺寸和 QLabel 尺寸
  int imgWidth = pixmap.width();
  int imgHeight = pixmap.height();
  int labelWidth = this->width();
  int labelHeight = this->height();
  int imgWidthInLabel;
  int imgHeightInLabel;

  /*
    图片大小不一，需要将图片缩放到合适大小，以在Label中显示
    根据宽高比
        imgWidth / imgHeight 和 labelWidth / labelHeight
    决定缩放比例

    若图片比label高度更高， 则有：
        imgWidth / imgHeight < labelWidth / labelHeight
    将上述不等式转为乘法形式（比除法效率高）：
        imgWidth * labelHeight < imgHeight * labelWidth

    此时以高度作为基准进行缩放：
    通过labelHeight/imgHeight确定缩放比例，避免缩放后图片
    高度不会超过label高度，导致图片上下部分显示不全


  */

  if (imgWidth * labelHeight < imgHeight * labelWidth) {
    // 图片相对 QLabel 更“高”，以高度为基准缩放
    scaledRate = labelHeight / double(imgHeight); // 缩放比例
    imgHeightInLabel = labelHeight;
    imgWidthInLabel = int(scaledRate * imgWidth); // 按比例计算宽度
    imageRect.setRect((labelWidth - imgWidthInLabel) / 2, 0, imgWidthInLabel,
                      imgHeightInLabel); // 图片居中
  } else {
    // 图片相对 QLabel 更“宽”，以宽度为基准缩放
    scaledRate = labelWidth / double(imgWidth); // 缩放比例
    imgWidthInLabel = labelWidth;
    imgHeightInLabel = int(scaledRate * imgHeight); // 按比例计算高度
    imageRect.setRect(0, (labelHeight - imgHeightInLabel) / 2, imgWidthInLabel,
                      imgHeightInLabel); // 居中显示
  }

  // 将原图缩放到 QLabel 上显示的尺寸
  tempImage =
      originalImage.scaled(imgWidthInLabel, imgHeightInLabel,
                           Qt::KeepAspectRatio, Qt::SmoothTransformation);
  this->setPixmap(tempImage); // 显示缩放后的图片

  // 如果裁剪框是固定尺寸，需要按缩放比例调整显示尺寸
  if (cropperShape >= CropperShape::FIXED_RECT) {
    cropperRect.setWidth(int(cropperRect_.width() * scaledRate));
    cropperRect.setHeight(int(cropperRect_.height() * scaledRate));
  }

  // 重置裁剪框位置到图片中心
  resetCropperPos();
}

/*****************************************
 * set cropper's shape (and size)
 *****************************************/
void ImageCropperLabel::setRectCropper() {
  cropperShape = CropperShape::RECT;
  resetCropperPos();
}

void ImageCropperLabel::setSquareCropper() {
  cropperShape = CropperShape::SQUARE;
  resetCropperPos();
}

void ImageCropperLabel::setEllipseCropper() {
  cropperShape = CropperShape::ELLIPSE;
  resetCropperPos();
}

void ImageCropperLabel::setCircleCropper() {
  cropperShape = CropperShape::CIRCLE;
  resetCropperPos();
}

void ImageCropperLabel::setFixedRectCropper(QSize size) {
  cropperShape = CropperShape::FIXED_RECT;
  cropperRect_.setSize(size);
  resetCropperPos();
}

void ImageCropperLabel::setFixedEllipseCropper(QSize size) {
  cropperShape = CropperShape::FIXED_ELLIPSE;
  cropperRect_.setSize(size);
  resetCropperPos();
}

// not recommended
void ImageCropperLabel::setCropper(CropperShape shape, QSize size) {
  cropperShape = shape;
  cropperRect_.setSize(size);
  resetCropperPos();
}

/*****************************************************************************
 * Set cropper's fixed size
 *****************************************************************************/
void ImageCropperLabel::setCropperFixedSize(int fixedWidth, int fixedHeight) {
  cropperRect_.setSize(QSize(fixedWidth, fixedHeight));
  resetCropperPos();
}

void ImageCropperLabel::setCropperFixedWidth(int fixedWidth) {
  cropperRect_.setWidth(fixedWidth);
  resetCropperPos();
}

void ImageCropperLabel::setCropperFixedHeight(int fixedHeight) {
  cropperRect_.setHeight(fixedHeight);
  resetCropperPos();
}

/**********************************************
 * Move cropper to the center of the image
 * And resize to default
 * 每次调用都让裁剪框回到可见区域中央
 **********************************************/
void ImageCropperLabel::resetCropperPos() {
  int labelWidth = this->width();
  int labelHeight = this->height();

  if (cropperShape == CropperShape::FIXED_RECT ||
      cropperShape == CropperShape::FIXED_ELLIPSE) {
    cropperRect.setWidth(int(cropperRect_.width() * scaledRate));
    cropperRect.setHeight(int(cropperRect_.height() * scaledRate));
  }

  switch (cropperShape) {
  case CropperShape::UNDEFINED:
    break;
  case CropperShape::FIXED_RECT:
  case CropperShape::FIXED_ELLIPSE: {
    cropperRect.setRect((labelWidth - cropperRect.width()) / 2,
                        (labelHeight - cropperRect.height()) / 2,
                        cropperRect.width(), cropperRect.height());
    break;
  }
  case CropperShape::RECT:
  case CropperShape::SQUARE:
  case CropperShape::ELLIPSE:
  case CropperShape::CIRCLE: {
    int imgWidth = tempImage.width();
    int imgHeight = tempImage.height();
    int edge = int((imgWidth > imgHeight ? imgHeight : imgWidth) * 3 / 4.0);
    cropperRect.setRect((labelWidth - edge) / 2, (labelHeight - edge) / 2, edge,
                        edge);
    break;
  }
  }
}

QPixmap ImageCropperLabel::getCroppedImage() {
  return getCroppedImage(this->outputShape);
}

QPixmap ImageCropperLabel::getCroppedImage(OutputShape shape) {
  /*
    cropperRect是在QLabel（显示层）上绘制的裁剪框，而原图尺寸比显示尺寸可能大或小。
    所以：
        cropperRect.left() - imageRect.left()
            得到裁剪框相对于显示图片左上角的偏移（显示坐标）
        除以 scaledRate
             转换为原始图片坐标
    然后在原始图片上拷贝对应区域的像素，得到裁剪后的图片
 */
  int startX = int((cropperRect.left() - imageRect.left()) / scaledRate);
  int startY = int((cropperRect.top() - imageRect.top()) / scaledRate);
  int croppedWidth = int(cropperRect.width() / scaledRate);
  int croppedHeight = int(cropperRect.height() / scaledRate);

  // 从原始图片中裁剪指定区域
  QPixmap resultImage(croppedWidth, croppedHeight);
  resultImage = originalImage.copy(startX, startY, croppedWidth, croppedHeight);

  // 如果输出为椭圆形，设置遮罩以裁剪成椭圆
  if (shape == OutputShape::ELLIPSE) {
    /*
        通过 QBitmap mask(size)
        创建一个与裁剪区域大小相同的二值遮罩（黑白图像）。
        drawRoundRect绘制圆角矩形，通过设定宽高一致和圆角弯曲度，使其看起来像椭圆
    */
    QSize size(croppedWidth, croppedHeight);
    QBitmap mask(size);
    QPainter painter(&mask);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 先填充白色背景
    painter.fillRect(0, 0, size.width(), size.height(), Qt::white);

    // 用黑色画刷绘制椭圆
    painter.setBrush(QColor(0, 0, 0));
    painter.drawRoundedRect(QRectF(0, 0, size.width(), size.height()), 99, 99,
                            Qt::AbsoluteSize);
    //QPixmap遮罩中，“黑色”区域映射为 保留原图像素，而把剩下的（白色）区域变成透明。
    // 应用遮罩，得到椭圆裁剪效果
    resultImage.setMask(mask);
  }

  return resultImage; // 返回裁剪后的图片
}

void ImageCropperLabel::paintEvent(QPaintEvent *event) {
    // 1. 先调用父类，实现原始图像的绘制
    QLabel::paintEvent(event);

    // 2. 根据当前裁剪形状，绘制不同的“半透明遮罩”或“高光边”
    switch (cropperShape) {
        case CropperShape::UNDEFINED:
            break;
        case CropperShape::FIXED_RECT:
            drawRectOpacity();
            break;
        case CropperShape::FIXED_ELLIPSE:
            drawEllipseOpacity();
            break;
        case CropperShape::RECT:
            drawRectOpacity();
            drawSquareEdge(!ONLY_FOUR_CORNERS);
            break;
        case CropperShape::SQUARE:
            drawRectOpacity();
            drawSquareEdge(ONLY_FOUR_CORNERS);
            break;
        case CropperShape::ELLIPSE:
            drawEllipseOpacity();
            drawSquareEdge(!ONLY_FOUR_CORNERS);
            break;
        case CropperShape::CIRCLE:
            drawEllipseOpacity();
            drawSquareEdge(ONLY_FOUR_CORNERS);
            break;
    }

    // 3. 如果需要，给裁剪框本身画一条边框
    if (isShowRectBorder) {
        QPainter painter(this);
        painter.setPen(borderPen);
        painter.drawRect(cropperRect);
    }
}

// 裁剪区域四个边角的区域高光（显示为小矩形），让用户知道可以拖拽这些边角矩形缩放裁剪区域
void ImageCropperLabel::drawSquareEdge(bool onlyFourCorners) {
  if (!isShowDragSquare)
    return;

  // Four corners
  drawFillRect(cropperRect.topLeft(), dragSquareEdge, dragSquareColor);
  drawFillRect(cropperRect.topRight(), dragSquareEdge, dragSquareColor);
  drawFillRect(cropperRect.bottomLeft(), dragSquareEdge, dragSquareColor);
  drawFillRect(cropperRect.bottomRight(), dragSquareEdge, dragSquareColor);

  // Four edges
  if (!onlyFourCorners) {
    int centralX = cropperRect.left() + cropperRect.width() / 2;
    int centralY = cropperRect.top() + cropperRect.height() / 2;
    drawFillRect(QPoint(cropperRect.left(), centralY), dragSquareEdge,
                 dragSquareColor);
    drawFillRect(QPoint(centralX, cropperRect.top()), dragSquareEdge,
                 dragSquareColor);
    drawFillRect(QPoint(cropperRect.right(), centralY), dragSquareEdge,
                 dragSquareColor);
    drawFillRect(QPoint(centralX, cropperRect.bottom()), dragSquareEdge,
                 dragSquareColor);
  }
}

void ImageCropperLabel::drawFillRect(QPoint centralPoint, int edge,
                                     QColor color) {
  QRect rect(centralPoint.x() - edge / 2, centralPoint.y() - edge / 2, edge,
             edge);
  QPainter painter(this);
  painter.fillRect(rect, color);
}

// Opacity effect
void ImageCropperLabel::drawOpacity(const QPainterPath &path) {
  QPainter painterOpac(this);
  painterOpac.setOpacity(opacity);
  painterOpac.fillPath(path, QBrush(Qt::black));
}

void ImageCropperLabel::drawRectOpacity() {
  if (isShowOpacityEffect) {
    QPainterPath p1, p2, p;
    p1.addRect(imageRect);
    p2.addRect(cropperRect);
    p = p1.subtracted(p2);
    drawOpacity(p);
  }
}

void ImageCropperLabel::drawEllipseOpacity() {
  if (isShowOpacityEffect) {
    QPainterPath p1, p2, p;
    p1.addRect(imageRect);
    p2.addEllipse(cropperRect);
    p = p1.subtracted(p2);
    drawOpacity(p);
  }
}

// 裁剪区域，四个边角的拖拽手柄位置检测
bool ImageCropperLabel::isPosNearDragSquare(const QPoint &pt1,
                                            const QPoint &pt2) {
  return abs(pt1.x() - pt2.x()) * 2 <= dragSquareEdge &&
         abs(pt1.y() - pt2.y()) * 2 <= dragSquareEdge;
}

int ImageCropperLabel::getPosInCropperRect(const QPoint &pt) {
  if (isPosNearDragSquare(
          pt, QPoint(cropperRect.right(), cropperRect.center().y())))
    return RECT_RIGHT;
  if (isPosNearDragSquare(pt, cropperRect.bottomRight()))
    return RECT_BOTTOM_RIGHT;
  if (isPosNearDragSquare(
          pt, QPoint(cropperRect.center().x(), cropperRect.bottom())))
    return RECT_BOTTOM;
  if (isPosNearDragSquare(pt, cropperRect.bottomLeft()))
    return RECT_BOTTOM_LEFT;
  if (isPosNearDragSquare(pt,
                          QPoint(cropperRect.left(), cropperRect.center().y())))
    return RECT_LEFT;
  if (isPosNearDragSquare(pt, cropperRect.topLeft()))
    return RECT_TOP_LEFT;
  if (isPosNearDragSquare(pt,
                          QPoint(cropperRect.center().x(), cropperRect.top())))
    return RECT_TOP;
  if (isPosNearDragSquare(pt, cropperRect.topRight()))
    return RECT_TOP_RIGHT;
  if (cropperRect.contains(pt, true))
    return RECT_INSIDE;
  return RECT_OUTSIZD;
}

/*************************************************
 *
 *  Change mouse cursor type
 *      Arrow, SizeHor, SizeVer, etc...
 *
 *************************************************/

void ImageCropperLabel::changeCursor() {
  switch (cursorPosInCropperRect) {
  case RECT_OUTSIZD:
    setCursor(Qt::ArrowCursor);
    break;
  case RECT_BOTTOM_RIGHT: {
    switch (cropperShape) {
    case CropperShape::SQUARE:
    case CropperShape::CIRCLE:
    case CropperShape::RECT:
    case CropperShape::ELLIPSE:
      setCursor(Qt::SizeFDiagCursor);
      break;
    default:
      break;
    }
    break;
  }
  case RECT_RIGHT: {
    switch (cropperShape) {
    case CropperShape::RECT:
    case CropperShape::ELLIPSE:
      setCursor(Qt::SizeHorCursor);
      break;
    default:
      break;
    }
    break;
  }
  case RECT_BOTTOM: {
    switch (cropperShape) {
    case CropperShape::RECT:
    case CropperShape::ELLIPSE:
      setCursor(Qt::SizeVerCursor);
      break;
    default:
      break;
    }
    break;
  }
  case RECT_BOTTOM_LEFT: {
    switch (cropperShape) {
    case CropperShape::RECT:
    case CropperShape::ELLIPSE:
    case CropperShape::SQUARE:
    case CropperShape::CIRCLE:
      setCursor(Qt::SizeBDiagCursor);
      break;
    default:
      break;
    }
    break;
  }
  case RECT_LEFT: {
    switch (cropperShape) {
    case CropperShape::RECT:
    case CropperShape::ELLIPSE:
      setCursor(Qt::SizeHorCursor);
      break;
    default:
      break;
    }
    break;
  }
  case RECT_TOP_LEFT: {
    switch (cropperShape) {
    case CropperShape::RECT:
    case CropperShape::ELLIPSE:
    case CropperShape::SQUARE:
    case CropperShape::CIRCLE:
      setCursor(Qt::SizeFDiagCursor);
      break;
    default:
      break;
    }
    break;
  }
  case RECT_TOP: {
    switch (cropperShape) {
    case CropperShape::RECT:
    case CropperShape::ELLIPSE:
      setCursor(Qt::SizeVerCursor);
      break;
    default:
      break;
    }
    break;
  }
  case RECT_TOP_RIGHT: {
    switch (cropperShape) {
    case CropperShape::SQUARE:
    case CropperShape::CIRCLE:
    case CropperShape::RECT:
    case CropperShape::ELLIPSE:
      setCursor(Qt::SizeBDiagCursor);
      break;
    default:
      break;
    }
    break;
  }
  case RECT_INSIDE: {
    setCursor(Qt::SizeAllCursor);
    break;
  }
  }
}

/*****************************************************
 *
 *  Mouse Events
 *
 *****************************************************/

void ImageCropperLabel::mousePressEvent(QMouseEvent *e) {
  currPos = lastPos = e->pos();
  isLButtonPressed = true;
}

void ImageCropperLabel::mouseMoveEvent(QMouseEvent *e) {
    currPos = e->pos();
    // 首次进入时，确定鼠标在哪个区域：边角、边缘、框内或框外
    if (!isCursorPosCalculated) {
        cursorPosInCropperRect = getPosInCropperRect(currPos);
        changeCursor();  // 根据区域切换不同形状的鼠标指针
    }

    // 如果鼠标左键未按下，不处理
    if (!isLButtonPressed)
        return;
    // 如果当前点不在图片区域内，不处理
    if (!imageRect.contains(currPos))
        return;

    isCursorPosCalculated = true;  // 保证只计算一次区域

    // 计算本次移动的偏移量
    int xOffset = currPos.x() - lastPos.x();
    int yOffset = currPos.y() - lastPos.y();
    lastPos = currPos;

    int disX = 0;
    int disY = 0;  // 用于后续缩放计算

    // 根据鼠标所在区域，选择对应的移动/缩放逻辑
    switch (cursorPosInCropperRect) {
        case RECT_OUTSIZD:
            break;  // 在框外：不处理

        // —— 右下角 缩放 ——
        case RECT_BOTTOM_RIGHT: {
            disX = currPos.x() - cropperRect.left();
            disY = currPos.y() - cropperRect.top();
            switch (cropperShape) {
                // 固定模式：不允许缩放
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                    break;
                // 正方形／圆形：强制保持宽高一致
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    setCursor(Qt::SizeFDiagCursor);
                    // 保证没有小于最小尺寸且不超出图片下/right 边
                    if (disX >= cropperMinimumWidth && disY >= cropperMinimumHeight) {
                        if (disX > disY && cropperRect.top() + disX <= imageRect.bottom()) {
                            // 宽度主导，伸长底边
                            cropperRect.setRight(currPos.x());
                            cropperRect.setBottom(cropperRect.top() + disX);
                            emit croppedImageChanged();
                        }
                        else if (disX <= disY && cropperRect.left() + disY <= imageRect.right()) {
                            // 高度主导，伸长右边
                            cropperRect.setBottom(currPos.y());
                            cropperRect.setRight(cropperRect.left() + disY);
                            emit croppedImageChanged();
                        }
                    }
                    break;
                // 普通矩形／椭圆：独立伸缩宽或高
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    setCursor(Qt::SizeFDiagCursor);
                    if (disX >= cropperMinimumWidth) {
                        cropperRect.setRight(currPos.x());
                        emit croppedImageChanged();
                    }
                    if (disY >= cropperMinimumHeight) {
                        cropperRect.setBottom(currPos.y());
                        emit croppedImageChanged();
                    }
                    break;
            }
            break;
        }

        // —— 右侧边 缩放 ——
        case RECT_RIGHT: {
            disX = currPos.x() - cropperRect.left();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    break;
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    if (disX >= cropperMinimumWidth) {
                        cropperRect.setRight(currPos.x());
                        emit croppedImageChanged();
                    }
                    break;
            }
            break;
        }

        // —— 底部边 缩放 ——
        case RECT_BOTTOM: {
            disY = currPos.y() - cropperRect.top();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    break;
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    if (disY >= cropperMinimumHeight) {
                        cropperRect.setBottom(cropperRect.bottom() + yOffset);
                        emit croppedImageChanged();
                    }
                    break;
            }
            break;
        }

        // —— 左下角 缩放 ——
        case RECT_BOTTOM_LEFT: {
            disX = cropperRect.right() - currPos.x();
            disY = currPos.y() - cropperRect.top();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                    break;
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    // 普通矩形/椭圆：独立调整宽高
                    if (disX >= cropperMinimumWidth) {
                        cropperRect.setLeft(currPos.x());
                        emit croppedImageChanged();
                    }
                    if (disY >= cropperMinimumHeight) {
                        cropperRect.setBottom(currPos.y());
                        emit croppedImageChanged();
                    }
                    break;
                // 正方形/圆形：保持宽高一致
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    if (disX >= cropperMinimumWidth && disY >= cropperMinimumHeight) {
                        if (disX > disY && cropperRect.top() + disX <= imageRect.bottom()) {
                            cropperRect.setLeft(currPos.x());
                            cropperRect.setBottom(cropperRect.top() + disX);
                            emit croppedImageChanged();
                        }
                        else if (disX <= disY && cropperRect.right() - disY >= imageRect.left()) {
                            cropperRect.setBottom(currPos.y());
                            cropperRect.setLeft(cropperRect.right() - disY);
                            emit croppedImageChanged();
                        }
                    }
                    break;
            }
            break;
        }

        // —— 左侧边 缩放 ——
        case RECT_LEFT: {
            disX = cropperRect.right() - currPos.x();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    break;
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    if (disX >= cropperMinimumHeight) {
                        cropperRect.setLeft(cropperRect.left() + xOffset);
                        emit croppedImageChanged();
                    }
                    break;
            }
            break;
        }

        // —— 左上角 缩放 ——
        case RECT_TOP_LEFT: {
            disX = cropperRect.right() - currPos.x();
            disY = cropperRect.bottom() - currPos.y();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                    break;
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    if (disX >= cropperMinimumWidth) {
                        cropperRect.setLeft(currPos.x());
                        emit croppedImageChanged();
                    }
                    if (disY >= cropperMinimumHeight) {
                        cropperRect.setTop(currPos.y());
                        emit croppedImageChanged();
                    }
                    break;
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    if (disX >= cropperMinimumWidth && disY >= cropperMinimumHeight) {
                        if (disX > disY && cropperRect.bottom() - disX >= imageRect.top()) {
                            cropperRect.setLeft(currPos.x());
                            cropperRect.setTop(cropperRect.bottom() - disX);
                            emit croppedImageChanged();
                        }
                        else if (disX <= disY && cropperRect.right() - disY >= imageRect.left()) {
                            cropperRect.setTop(currPos.y());
                            cropperRect.setLeft(cropperRect.right() - disY);
                            emit croppedImageChanged();
                        }
                    }
                    break;
            }
            break;
        }

        // —— 上边 缩放 ——
        case RECT_TOP: {
            disY = cropperRect.bottom() - currPos.y();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    break;
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    if (disY >= cropperMinimumHeight) {
                        cropperRect.setTop(cropperRect.top() + yOffset);
                        emit croppedImageChanged();
                    }
                    break;
            }
            break;
        }

        // —— 右上角 缩放 ——
        case RECT_TOP_RIGHT: {
            disX = currPos.x() - cropperRect.left();
            disY = cropperRect.bottom() - currPos.y();
            switch (cropperShape) {
                case CropperShape::UNDEFINED:
                case CropperShape::FIXED_RECT:
                case CropperShape::FIXED_ELLIPSE:
                    break;
                case CropperShape::RECT:
                case CropperShape::ELLIPSE:
                    if (disX >= cropperMinimumWidth) {
                        cropperRect.setRight(currPos.x());
                        emit croppedImageChanged();
                    }
                    if (disY >= cropperMinimumHeight) {
                        cropperRect.setTop(currPos.y());
                        emit croppedImageChanged();
                    }
                    break;
                case CropperShape::SQUARE:
                case CropperShape::CIRCLE:
                    if (disX >= cropperMinimumWidth && disY >= cropperMinimumHeight) {
                        if (disX < disY && cropperRect.left() + disY <= imageRect.right()) {
                            cropperRect.setTop(currPos.y());
                            cropperRect.setRight(cropperRect.left() + disY);
                            emit croppedImageChanged();
                        }
                        else if (disX >= disY && cropperRect.bottom() - disX >= imageRect.top()) {
                            cropperRect.setRight(currPos.x());
                            cropperRect.setTop(cropperRect.bottom() - disX);
                            emit croppedImageChanged();
                        }
                    }
                    break;
            }
            break;
        }

        // —— 框内拖动 ——
        case RECT_INSIDE: {
            // 先检测移动后是否会超出图片范围，将偏移量 xOffset/yOffset 裁剪到合法区间
            if (xOffset > 0) {
                if (cropperRect.right() + xOffset > imageRect.right())
                    xOffset = 0;
            }
            else if (xOffset < 0) {
                if (cropperRect.left() + xOffset < imageRect.left())
                    xOffset = 0;
            }
            if (yOffset > 0) {
                if (cropperRect.bottom() + yOffset > imageRect.bottom())
                    yOffset = 0;
            }
            else if (yOffset < 0) {
                if (cropperRect.top() + yOffset < imageRect.top())
                    yOffset = 0;
            }
            // 移动整个裁剪框
            cropperRect.moveTo(cropperRect.left() + xOffset, cropperRect.top() + yOffset);
            emit croppedImageChanged();
            break;
        }
    }

    repaint();  // 触发重绘，及时在界面上更新新的裁剪框
}

void ImageCropperLabel::mouseReleaseEvent(QMouseEvent *) {
  isLButtonPressed = false;
  isCursorPosCalculated = false;
  setCursor(Qt::ArrowCursor);
}


