#ifndef TCP_MANAGER_HPP
#define TCP_MANAGER_HPP
#include "client_struct_def.hpp"
#include "utility/constant.hpp"
#include "utility/singleton.hpp"
#include <QTcpSocket>
class SearchInfo;
class AddFriendApply;
class AuthRsp;
class AuthInfo;
struct TextChatMsg;
class TcpMgr : public QObject,
               public Singleton<TcpMgr>,
               public std::enable_shared_from_this<TcpMgr> {
  Q_OBJECT
public:
  TcpMgr();
public slots:
  void slot_tcp_connect(ServerInfo);
  void slot_send_data(ReqId reqId, QByteArray data);
signals:
  void sig_con_success(bool bsuccess);
  void sig_send_data(ReqId reqId, QByteArray data);
  void sig_login_failed(int error);
  void sig_switch_chatdlg();
  // 搜索用户
  void sig_user_search(std::shared_ptr<SearchInfo>);
  // 申请添加对方为好友
  void sig_recv_friend_apply(std::shared_ptr<AddFriendApply>);
  // 处理完对方好友申请
  void sig_friend_apply_rsp(std::shared_ptr<AuthRsp>);
  // 对方处理完好友申请的答复
  void sig_recv_friend_auth(std::shared_ptr<AuthInfo>);
  // 收到他人消息的信号, 登录时可能收到多条消息
  void sig_recv_text_msg(std::shared_ptr<TextChatMsg> msg);

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