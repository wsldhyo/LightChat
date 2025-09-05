#ifndef CLIENT_STRUCT_DEF_HPP
#define CLIENT_STRUCT_DEF_HPP
#include <QString>
#include <QPixmap>
struct ServerInfo {
  QString Host;
  QString Port;
  QString Token;
  int Uid;
};


struct MsgInfo{
    QString msgFlag;//"text,image,file"
    QString content;//表示文件和图像的url,文本信息
    QPixmap pixmap;//文件和图片的缩略图
};
#endif