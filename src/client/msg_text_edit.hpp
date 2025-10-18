#ifndef MSG_TEXT_EDIT_HPP
#define MSG_TEXT_EDIT_HPP
#include "client_struct_def.hpp"
#include <QApplication>
#include <QDrag>
#include <QFileIconProvider>
#include <QFileInfo>
#include <QMimeData>
#include <QMimeType>
#include <QMouseEvent>
#include <QObject>
#include <QPainter>
#include <QTextEdit>
#include <QVector>
/**
 * @brief 聊天界面ChatPage中的的消息编辑框
 *
 */
class MessageTextEdit : public QTextEdit {
  Q_OBJECT
public:
  explicit MessageTextEdit(QWidget *parent = nullptr);

  ~MessageTextEdit();

  QVector<MsgInfo> get_msg_list();

  void insert_file_from_url(const QStringList &urls);
signals:
  void send();

protected:
  void dragEnterEvent(QDragEnterEvent *event) override;
  void dropEvent(QDropEvent *event) override;
  void keyPressEvent(QKeyEvent *e) override;

private:
  bool canInsertFromMimeData(const QMimeData *source) const override;
  void insertFromMimeData(const QMimeData *source) override;

  void insert_images(const QString &url);
  void insert_text_file(const QString &url);

private:
  bool is_image(QString url); //判断文件是否为图片
  void insert_msg_list(QVector<MsgInfo> &list, QString flag, QString text,
                     QPixmap pix);

  QStringList get_url(QString text);
  QPixmap
  get_file_icon_pixmap(const QString &url); //获取文件图标及大小信息，并转化成图片
  QString get_file_size(qint64 size); //获取文件大小

private slots:
  void slot_text_edit_changed();

private:
  QVector<MsgInfo> msg_list_;    // 发送消息缓冲
  QVector<MsgInfo> get_msg_list_; // 接受消息缓冲
};

#endif