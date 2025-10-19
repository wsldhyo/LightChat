#ifndef IMAGE_CROPPER_DLG_HPP
#define IMAGE_CROPPER_DLG_HPP
#include "image_cropper_lbl.hpp"
#include <QDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QPixmap>
#include <QPushButton>
#include <QString>
#include <QVBoxLayout>
#include <QWidget>

/**
 * @class ImageCropperDlgPrivate
 * @brief 弹出式图片裁剪对话框私有类。
 *
 * 功能：
 *  - 内部持有 ImageCropperLabel，用于显示图片和裁剪框。
 *  - 提供 OK / Cancel 按钮，裁剪完成后通过 outputImage 返回结果。
 */
class ImageCropperDlgPrivate : public QDialog {
  Q_OBJECT
public:
  /**
   * @brief 构造函数
   * @param imageIn 输入图片
   * @param outputImage 裁剪结果引用
   * @param windowWidth 对话框宽度
   * @param windowHeight 对话框高度
   * @param shape 裁剪框形状
   * @param cropperSize 裁剪框固定尺寸（可选）
   */
  ImageCropperDlgPrivate(const QPixmap &imageIn, QPixmap &outputImage,
                         int windowWidth, int windowHeight, CropperShape shape,
                         QSize cropperSize = QSize());

private:
  ImageCropperLabel *imageLabel; /**< 显示图片和裁剪框 */
  QPushButton *btnOk;            /**< 确认按钮 */
  QPushButton *btnCancel;        /**< 取消按钮 */
  QPixmap &outputImage;          /**< 裁剪结果引用 */
};

/**
 * @class ImageCropperDlg
 * @brief 静态工具类，用于弹出裁剪对话框并获取裁剪结果。
 */
class ImageCropperDlg : QObject {
public:
  /**
   * @brief 弹出裁剪对话框并返回裁剪结果
   * @param filename 输入图片路径
   * @param windowWidth 对话框宽度
   * @param windowHeight 对话框高度
   * @param cropperShape 裁剪框形状
   * @param crooperSize 裁剪框固定尺寸（可选）
   * @return 裁剪后的 QPixmap
   */
  static QPixmap getCroppedImage(const QString &filename, int windowWidth,
                                 int windowHeight, CropperShape cropperShape,
                                 QSize crooperSize = QSize());
};
#endif // IMAGE_CROPPER_DLG_HPP