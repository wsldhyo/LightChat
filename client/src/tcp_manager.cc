#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>

#include "tcp_manager.hpp"
#include "user_manager.hpp"
TcpManager::TcpManager()
    : host_(""), port_(0), b_recv_pending_(false), message_id_(0),
      message_len_(0) {
  create_connection();
}

void TcpManager::slot_tcp_connect(ServerInfo _si) {
  // 尝试连接到服务器
  qDebug() << "Try Connecting to server...";
  host_ = _si.host;
  port_ = static_cast<uint16_t>(_si.port.toUInt());
  socket_.connectToHost(_si.host, port_);
}

// 客户端发送数据可能在任何线程，后期利用Qt的信号槽队列机制保证线程安全
// 发送数据给服务器
void TcpManager::slot_send_data(RequestID _req_ID, QString _data) {
  uint16_t id = static_cast<uint16_t>(_req_ID);
  QByteArray data_bytes = _data.toUtf8();

  // 检查数据长度是否超出 quint16 范围
  if (data_bytes.size() > std::numeric_limits<quint16>::max()) {
    qWarning() << "Data exceeds maximum allowed size (65535 bytes).";
    return;
  }
  quint16 len = static_cast<quint16>(data_bytes.size());

  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);
  out.setVersion(QDataStream::Qt_5_15); // 设置版本
  out.setByteOrder(QDataStream::BigEndian);
  // 写入tlv格式数据
  out << id << len;
  out.writeBytes(data_bytes.constData(), data_bytes.size()); // 写入原始字节

  // 发送数据并检查结果
  qint64 bytesWritten = socket_.write(block);
  if (bytesWritten == -1) {
    qDebug() << "Write error:" << socket_.errorString();
  } else if (bytesWritten < block.size()) {
    qDebug() << "Data partially sent, remaining:"
             << block.size() - bytesWritten;
  }
}

void TcpManager::slot_handle_tcp_error(QAbstractSocket::SocketError _error) {

  qDebug() << "Error:" << socket_.errorString();
  switch (_error) {
  case QTcpSocket::ConnectionRefusedError:
    qDebug() << "Connection Refused!";
    emit sig_connect_success(false);
    break;
  case QTcpSocket::RemoteHostClosedError:
    qDebug() << "Remote Host Closed Connection!";
    break;
  case QTcpSocket::HostNotFoundError:
    qDebug() << "Host Not Found!";
    emit sig_connect_success(false);
    break;
  case QTcpSocket::SocketTimeoutError:
    qDebug() << "Connection Timeout!";
    emit sig_connect_success(false);
    break;
  case QTcpSocket::NetworkError:
    qDebug() << "Network Error!";
    break;
  default:
    qDebug() << "Other Error!";
    break;
  }
}

void TcpManager::slot_read_data() {
  // 当有数据可读时，读取所有数据，
  // Qt的是非阻塞的，只要读到数据（可能读不全）就会发射信号

  buffer_.append(socket_.readAll()); // 追加TCP读取数据到缓冲区
  QDataStream stream(&buffer_, QIODevice::ReadOnly);
  stream.setVersion(QDataStream::Qt_5_15);

  // 对buffer_缓冲区的数据进行切包处理
  forever {
    // 先解析头部
    if (!b_recv_pending_) {
      // 检查缓冲区中的数据是否足够解析出一个消息头（消息ID + 消息长度）
      if (buffer_.size() < static_cast<int>(sizeof(quint16) * 2)) {
        return; // 数据不够，等待更多数据
      }
      // 预读取消息ID和消息长度，但不从缓冲区中移除
      stream >> message_id_ >> message_len_;
      // 将buffer 中的前四个字节移除
      buffer_ = buffer_.mid(sizeof(quint16) * 2);
      // 输出读取的数据
      qDebug() << "Message ID:" << message_id_ << ", Length:" << message_len_;
    }
    // buffer剩余长读是否满足消息体长度，不满足则退出继续等待接受
    if (buffer_.size() < message_len_) {
      b_recv_pending_ = true;
      return;
    }
    b_recv_pending_ = false; // 下一个消息体头部解析标志

    // 读取消息体
    QByteArray messageBody = buffer_.mid(0, message_len_);
    qDebug() << "receive body msg is " << messageBody;
    buffer_ = buffer_.mid(message_len_);
  }
}

void TcpManager::init_handlers() {
    // init_handlers是在构造函数里调用的，这时候对象还没构造完，所以回调不不捕获self，利用Qt对象树管理生命周期
    //auto self = shared_from_this();
  
  
  handlers_.insert(RequestID::LOGIN_CHAT_SERVER_RSP,
                   [this](RequestID _id, int _len, QByteArray data) {
                     Q_UNUSED(_len);
                     qDebug() << "handle _id is " << static_cast<int>(_id);

                     // 将QByteArray转换为QJsonDocument
                     QJsonDocument jsonDoc = QJsonDocument::fromJson(data);

                     // 检查转换是否成功
                     if (jsonDoc.isNull()) {
                       qDebug() << "Failed to create QJsonDocument.";
                       return;
                     }

                     QJsonObject jsonObj = jsonDoc.object();
                     qDebug() << "data jsonobj is " << jsonObj;

                     // 检查回包中是否包含error字段，没有该字段说明回包的json格式错误
                     if (!jsonObj.contains("error")) {
                       auto err = ErrorCode::JSON_PARSE_FAILED;
                       qDebug() << "Login Failed, err is Json Parse Err"
                                << static_cast<int>(err);
                       emit sig_login_failed(err);
                       return;
                     }

                     // 检查回包中的错误标记
                     int err = jsonObj["error"].toInt();
                     if (err != static_cast<int>(ErrorCode::NO_ERROR)) {
                       qDebug() << "Login Failed, err is " << err;
                       // 发射信号，处理登录错误
                       emit sig_login_failed(static_cast<ErrorCode>(err));
                       return;
                     }

                     auto uid = jsonObj["uid"].toInt();
                     auto name = jsonObj["name"].toString();
                     auto token = jsonObj["token"].toString();
                     //  auto user_info = std::make_shared<UserInfo>(uid, name,
                     //  nick, icon, sex);

                    UserManager::get_instance()->set_name(name);
                    UserManager::get_instance()->set_uid(uid);
                    UserManager::get_instance()->set_token(token);
                     //  UserMgr::GetInstance()->SetUserInfo(user_info);
                     //  UserMgr::GetInstance()->SetToken(jsonObj["token"].toString());
                     //  if (jsonObj.contains("apply_list")) {
                     //    UserMgr::GetInstance()->AppendApplyList(jsonObj["apply_list"].toArray());
                     //  }

                     //  // 添加好友列表
                     //  if (jsonObj.contains("friend_list")) {
                     //    UserMgr::GetInstance()->AppendFriendList(
                     //        jsonObj["friend_list"].toArray());
                     //  }

                     emit sig_switch_chat_dlg();
                   });
}
void TcpManager::create_connection() {

  QObject::connect(&socket_, &QTcpSocket::connected, [&]() {
    // slot_tcp_connect连接服务器成功后，打印消息，并发射信号通知服务器连接成功
    qDebug() << "Connected to server!";
    // 连接建立后发送消息
    emit sig_connect_success(true);
  });
  // 处理连接断开
  QObject::connect(&socket_, &QTcpSocket::disconnected,
                   [&]() { qDebug() << "Disconnected from server."; });

  QObject::connect(this, &TcpManager::sig_send_data, this,
                   &TcpManager::slot_send_data);

  QObject::connect(&socket_, &QTcpSocket::readyRead, this, 
                   &TcpManager::slot_read_data);
  //  处理错误5.15 之后版本
  // QTcpSocket::error 信号被标记为废弃，使用更明确的 errorOccurred。
  // errorOccurred 是单一信号，无需像旧版本那样通过 static_cast 选择重载版本。
  QObject::connect(&socket_, &QTcpSocket::errorOccurred, this,
                   &TcpManager::slot_handle_tcp_error);
  // 处理错误（适用于Qt 5.15之前的版本）
  //  QObject::connect(&socket_,
  //                   static_cast<void
  //                   (QTcpSocket::*)(QTcpSocket::SocketError)>(
  //                       &QTcpSocket::error),
  //                   &TcpManager::slot_handle_tcp_error);
}

// 查找handlers_里的回调函数，处理相应的服务器消息响应
void TcpManager::handle_msg(RequestID _id, int _len, QByteArray _data) {
  auto find_iter = handlers_.find(_id);
  if (find_iter == handlers_.end()) {
    qDebug() << "not found _id [" << static_cast<int>(_id) << "] to handle";
    return;
  }

  find_iter.value()(_id, _len, _data);
}