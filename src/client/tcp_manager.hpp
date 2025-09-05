#ifndef TCP_MANAGER_HPP
#define TCP_MANAGER_HPP
#include "client_struct_def.hpp"
#include "utility/constant.hpp"
#include "utility/singleton.hpp"
#include <QTcpSocket>
class SearchInfo;
class TcpMgr : public QObject,
               public Singleton<TcpMgr>,
               public std::enable_shared_from_this<TcpMgr> {
  Q_OBJECT
public:
  TcpMgr();
public slots:
  void slot_tcp_connect(ServerInfo);
  void slot_send_data(ReqId reqId, QString data);
signals:
  void sig_con_success(bool bsuccess);
  void sig_send_data(ReqId reqId, QString data);
  void sig_login_failed(int error);
  void sig_swich_chatdlg();
  void sig_user_search(std::shared_ptr<SearchInfo>);

private:
  void initHandlers();
  void handle_msg(ReqId id, int len, QByteArray data);

private:
  QTcpSocket socket_;
  QString host_;
  uint16_t port_;
  QByteArray buffer_;
  bool b_recv_pending_;
  quint16 message_id_;
  quint16 message_len_;
  QMap<ReqId, std::function<void(ReqId id, int len, QByteArray data)>>
      handlers_;
};

#endif // TCPMGR_H