#ifndef CLIENT_STRUCT_DEF_HPP
#define CLIENT_STRUCT_DEF_HPP
#include <QString>
#include <QPixmap>
struct ServerInfo {
  QString host;
  QString port;
  QString token;
  int uid;
};


struct MsgInfo{
    QString msg_flag;// 消息类型，text：文本,image：图片,file：文件
    QString content;//表示文件和图像的url,文本信息
    QPixmap pixmap;//文件和图片的缩略图
};



#endif