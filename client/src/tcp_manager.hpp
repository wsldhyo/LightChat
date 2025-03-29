#ifndef TCP_MANAGER_HPP
#define TCP_MANAGER_HPP
#include "../common/constant.hpp"
#include "../common/singleton.hpp"
#include "struct_def.hpp"
#include <QMap>
#include <QObject>
#include <QTcpSocket>
#include <memory>
/**
 * @brief
 *  Tcp长连接的管理类，用于维护聊天会话
 */
class TcpManager : public QObject,
                   public Singleton<TcpManager>,
                   public std::enable_shared_from_this<TcpManager> {
  Q_OBJECT
  friend class Singleton<TcpManager>;

public:
  TcpManager();
public slots:
  void slot_tcp_connect(ServerInfo);
  void slot_send_data(RequestID _req_ID, QString _data);
signals:
  // 通知其他界面连接成功
  void sig_connect_success(bool bsuccess);
  void sig_send_data(RequestID _reqID, QString _data);
  void sig_login_failed(ErrorCode _err);
  void sig_switch_chat_dlg();
private slots:
  void slot_handle_tcp_error(QAbstractSocket::SocketError _error);
  void slot_read_data();

private:
  void init_handlers();
  void create_connection();
  void handle_msg(RequestID _id, int _len, QByteArray _data);

  QTcpSocket socket_;
  QString host_;
  uint16_t port_;
  QByteArray buffer_;
  bool b_recv_pending_;
  quint16 message_id_;
  quint16 message_len_;

  QMap<RequestID, std::function<void(RequestID, int, QByteArray)>> handlers_;
};
#endif