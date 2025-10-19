#include "image_cropper_dlg.hpp"
ImageCropperDlgPrivate::ImageCropperDlgPrivate(
    const QPixmap &imageIn, QPixmap &outputImage, int windowWidth,
    int windowHeight, CropperShape shape, QSize cropperSize)
    : QDialog(nullptr), outputImage(outputImage) {
  
  // 对话框属性：模态、自动删除对象、鼠标追踪
  this->setAttribute(Qt::WA_DeleteOnClose, true);
  this->setWindowTitle("Image Cropper");
  this->setMouseTracking(true);
  this->setModal(true);

  // 创建图片裁剪控件并设置初始参数
  imageLabel = new ImageCropperLabel(windowWidth, windowHeight, this);
  imageLabel->setCropper(shape, cropperSize);
  imageLabel->setOutputShape(OutputShape::RECT); // 裁剪为正方形
  imageLabel->setOriginalImage(imageIn);
  imageLabel->enableOpacity(true);  //// 启用裁剪区域阴影效果

  // 创建按钮布局
  QHBoxLayout *btnLayout = new QHBoxLayout();
  btnOk = new QPushButton("OK", this);
  btnCancel = new QPushButton("Cancel", this);
  btnLayout->addStretch();
  btnLayout->addWidget(btnOk);
  btnLayout->addWidget(btnCancel);

  // 主布局：图片控件 + 按钮布局
  QVBoxLayout *mainLayout = new QVBoxLayout(this);
  mainLayout->addWidget(imageLabel);
  mainLayout->addLayout(btnLayout);

  // 按钮点击处理：OK 获取裁剪结果，Cancel 清空并关闭
  connect(btnOk, &QPushButton::clicked, this, [this]() {
    this->outputImage = this->imageLabel->getCroppedImage();
    this->close();
  });
  connect(btnCancel, &QPushButton::clicked, this, [this]() {
    this->outputImage = QPixmap();
    this->close();
  });
}


QPixmap ImageCropperDlg::getCroppedImage(const QString &filename,
                                         int windowWidth, int windowHeight,
                                         CropperShape cropperShape,
                                         QSize crooperSize) {
  QPixmap inputImage, outputImage;

  // 加载图片失败则提示并返回空结果
  if (!inputImage.load(filename)) {
    QMessageBox::critical(nullptr, "Error", "Load image failed!",
                          QMessageBox::Ok);
    return outputImage;
  }

  // 创建裁剪对话框并执行，阻塞直到用户确认或取消
  ImageCropperDlgPrivate *imageCropperDo =
      new ImageCropperDlgPrivate(inputImage, outputImage, windowWidth,
                                 windowHeight, cropperShape, crooperSize);
  imageCropperDo->exec();

  return outputImage;  // 返回裁剪结果
}
