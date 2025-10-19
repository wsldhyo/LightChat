#ifndef IMAGE_CROPPER_LBL_HPP
#define IMAGE_CROPPER_LBL_HPP
#include "client_constant.hpp"
#include <QLabel>
#include <QPen>
#include <QPixmap>

/**
 * @class ImageCropperLabel
 * @brief 可交互图片裁剪控件，继承自 QLabel。
 *
 * 功能：
 *  - 显示原始图片，并根据 QLabel 大小进行缩放。
 *  - 支持多种裁剪框形状：矩形、正方形、椭圆、圆形，以及固定尺寸矩形/椭圆。
 *  - 可拖动裁剪框调整位置和大小，支持鼠标交互。
 *  - 支持半透明遮罩效果，突出裁剪区域。
 *  - 提供获取裁剪后图片的接口。
 *
 * 计算说明：
 *  - 使用 scaledRate 将 QLabel 内裁剪框坐标转换为原图像素坐标。
 *  - 裁剪框默认居中，固定尺寸裁剪框根据 scaledRate 缩放。
 */
class ImageCropperLabel : public QLabel {
  Q_OBJECT
public:
  /**
   * @brief 构造函数，设置标签尺寸和鼠标跟踪。
   * @param width QLabel 宽度
   * @param height QLabel 高度
   * @param parent 父控件
   */
  ImageCropperLabel(int width, int height, QWidget *parent);

  /**
   * @brief 设置原始图片，并计算 QLabel 内显示矩形。
   * @param pixmap 原始 QPixmap
   *
   * 逻辑：
   *  - 按 QLabel 尺寸缩放图片，保持纵横比。
   *  - 计算 QLabel 内显示区域 imageRect。
   *  - 计算缩放比例 scaledRate，用于裁剪坐标换算。
   */
  void setOriginalImage(const QPixmap &pixmap);

  /**
   * @brief 设置输出裁剪形状。
   * @param shape 输出形状 RECT 或 ELLIPSE
   */
  void setOutputShape(OutputShape shape) { outputShape = shape; }

  /**
   * @brief 获取裁剪后的图片（默认输出形状）。
   * @return 裁剪后的 QPixmap
   */
  QPixmap getCroppedImage();

  /**
   * @brief 获取裁剪后的图片
   *
   * 根据当前裁剪框（cropperRect）在 QLabel
   * 上的位置和缩放比例，从原始图片中裁剪出对应区域。
   * 如果输出形状为椭圆（OutputShape::ELLIPSE），会对裁剪结果应用椭圆遮罩。
   *
   * @param shape 输出图片形状，RECT 或 ELLIPSE
   * @return QPixmap 裁剪后的图片
   *
   * 计算流程：
   * 1. 将 cropperRect 在 QLabel 上的坐标映射回原图坐标（除以 scaledRate）。
   * 2. 从 originalImage 中裁剪对应矩形区域。
   * 3. 如果输出为椭圆，则创建遮罩并应用，得到椭圆裁剪效果。
   */
  QPixmap getCroppedImage(OutputShape shape);

  /*****************************************
   * Set cropper's shape
   *****************************************/
  void setRectCropper();
  void setSquareCropper();
  void setEllipseCropper();
  void setCircleCropper();
  void setFixedRectCropper(QSize size);
  void setFixedEllipseCropper(QSize size);
  void setCropper(CropperShape shape, QSize size); // not recommended

  /*****************************************************************************
   * Set cropper's fixed size
   *****************************************************************************/
  void setCropperFixedSize(int fixedWidth, int fixedHeight);
  void setCropperFixedWidth(int fixedWidht);
  void setCropperFixedHeight(int fixedHeight);

  /*****************************************************************************
   * Set cropper's minimum size
   * default: twice the edge length of drag square
   *****************************************************************************/
  void setCropperMinimumSize(int minWidth, int minHeight) {
    cropperMinimumWidth = minWidth;
    cropperMinimumHeight = minHeight;
  }
  void setCropperMinimumWidth(int minWidth) { cropperMinimumWidth = minWidth; }
  void setCropperMinimumHeight(int minHeight) {
    cropperMinimumHeight = minHeight;
  }

  /*************************************************
   * Set the size, color, visibility of rectangular border
   *************************************************/
  void setShowRectBorder(bool show) { isShowRectBorder = show; }
  QPen getBorderPen() { return borderPen; }
  void setBorderPen(const QPen &pen) { borderPen = pen; }

  /*************************************************
   * Set the size, color of drag square
   *************************************************/
  void setShowDragSquare(bool show) { isShowDragSquare = show; }
  void setDragSquareEdge(int edge) { dragSquareEdge = (edge >= 3 ? edge : 3); }
  void setDragSquareColor(const QColor &color) { dragSquareColor = color; }

  /*****************************************
   *  Opacity Effect
   *****************************************/
  void enableOpacity(bool b = true) { isShowOpacityEffect = b; }
  void setOpacity(double newOpacity) { opacity = newOpacity; }

signals:
  /**
   * @brief 裁剪框改变时触发的信号
   */
  void croppedImageChanged();

protected:
  /*****************************************
   * Event handlers
   *****************************************/
  virtual void paintEvent(QPaintEvent *event) override;
  virtual void mousePressEvent(QMouseEvent *e) override;

  /*
    首次定位 当鼠标首次进入 mouseMoveEvent，用 getPosInCropperRect(currPos)
    判断鼠标在裁剪框的哪个“热区”——外部、框内、四边、四角中的哪一个，并调用
    changeCursor()
    切换对应的鼠标指针样式（如移动箭头、水平/垂直/对角调整形状等），以提示用户下一步操作。

    左右、上下、四角缩放

    对于矩形/椭圆，宽高可独立调整；
    对于正方形/圆，则保证 width == height，并根据位移量较大的一边来驱动另一边；
    对于“固定”模式，则完全不允许用户改变大小。
    边界与最小尺寸约束

    缩放时先判断新的宽度/高度是否 ≥ cropperMinimumWidth/Height；
    再判断新坐标是否会跑出 imageRect（图片区域）之外；
    最后才更新 cropperRect 并发信号 croppedImageChanged() 以便上层 UI
    或逻辑更新裁剪后的图像。 拖动整个裁剪框

    鼠标在框内部拖动（RECT_INSIDE），计算每次的偏移 xOffset,yOffset，
    并先“裁剪”偏移量，使整个框保持在图片范围内，
    最后调用 translate() 平移 cropperRect。
  */
  virtual void mouseMoveEvent(QMouseEvent *e) override;
  virtual void mouseReleaseEvent(QMouseEvent *e) override;

private:
  /***************************************
   * Draw shapes
   ***************************************/
  void drawFillRect(QPoint centralPoint, int edge,
                    QColor color); /**< 绘制小方块（拖动点） */
  void drawRectOpacity();          /**< 绘制矩形裁剪框半透明阴影 */
  void drawEllipseOpacity();       /**< 绘制椭圆裁剪框半透明阴影 */
  void drawOpacity(const QPainterPath &path); /**< 绘制阴影效果 */
  void drawSquareEdge(bool onlyFourCorners); /**< 绘制裁剪框拖动方块 */

  /***************************************
   * Utility methods
   ***************************************/

  /*
    在 鼠标按下 或 移动 时，调用
    getPosInCropperRect(pt)，能够快速定位出当前点相对于裁剪框的位置类型。
    上层逻辑（如鼠标事件处理）根据这个返回值，决定要进行哪种操作：
        如果是某个角或边的手柄，就进入“调整大小”模式，且拖拽方向锁定；
        如果是 RECT_INSIDE，则进入“移动整个裁剪框”模式；
        如果是 RECT_OUTSIZD，则不做任何裁剪框相关的拖拽操作。
  */
  int getPosInCropperRect(const QPoint &pt); /**< 判断鼠标位置在裁剪框哪部分 */
  bool isPosNearDragSquare(const QPoint &pt1,
                           const QPoint &pt2); /**< 判断点是否接近拖动小方块 */
  void resetCropperPos(); /**< 重置裁剪框位置和大小 */
  void changeCursor();    /**< 根据鼠标位置更新光标形状 */

  enum {
    RECT_OUTSIZD = 0,
    RECT_INSIDE = 1,
    RECT_TOP_LEFT,
    RECT_TOP,
    RECT_TOP_RIGHT,
    RECT_RIGHT,
    RECT_BOTTOM_RIGHT,
    RECT_BOTTOM,
    RECT_BOTTOM_LEFT,
    RECT_LEFT
  };

  const bool ONLY_FOUR_CORNERS = true;

private:
  QPixmap originalImage; /**< 原始图片 */
  QPixmap tempImage;     /**< 缩放后的图片，用于 QLabel 显示 */

  bool isShowRectBorder = true; /**< 是否显示裁剪框边框 */
  QPen borderPen;               /**< 边框样式 */

  CropperShape cropperShape = CropperShape::UNDEFINED; /**< 当前裁剪框形状 */
  OutputShape outputShape = OutputShape::RECT; /**< 输出裁剪形状 */

  QRect imageRect;    /**< 图片在 QLabel 中显示的矩形（缩放后） */
  QRect cropperRect;  /**< 裁剪框在 QLabel 中矩形（缩放后） */
  QRect cropperRect_; /**< 裁剪框真实尺寸（原图对应尺寸） */
  double scaledRate = 1.0; /**< 图片缩放比例 */

  bool isLButtonPressed = false;             /**< 左键是否按下 */
  bool isCursorPosCalculated = false;        /**< 鼠标位置是否已计算 */
  int cursorPosInCropperRect = RECT_OUTSIZD; /**< 鼠标在裁剪框的位置枚举 */
  QPoint lastPos;                            /**< 上一次鼠标位置 */
  QPoint currPos;                            /**< 当前鼠标位置 */

  bool isShowDragSquare = true;       /**< 是否显示拖动方块 */
  int dragSquareEdge = 8;             /**< 拖动方块边长 */
  QColor dragSquareColor = Qt::white; /**< 拖动方块颜色 */

  int cropperMinimumWidth = dragSquareEdge * 2;  /**< 裁剪框最小宽度 */
  int cropperMinimumHeight = dragSquareEdge * 2; /**< 裁剪框最小高度 */

  bool isShowOpacityEffect = false; /**< 是否显示半透明遮罩 */
  double opacity = 0.6;             /**< 遮罩透明度 */
};

#endif // IMAGE_CROPPER_LBL_HPP