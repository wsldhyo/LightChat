#include "tcp_manager.hpp"
#include "usermgr.hpp"
#include <QDataStream>
#include <QJsonDocument>
#include <QJsonObject>
TcpMgr::TcpMgr()
    : host_(""), port_(0), b_recv_pending_(false), message_id_(0),
      message_len_(0) {
  QObject::connect(&socket_, &QTcpSocket::connected, [&]() {
    qDebug() << "Connected to server!";
    // 连接建立后发送消息
    emit sig_con_success(true);
  });

  QObject::connect(&socket_, &QTcpSocket::readyRead, [&]() {
    // 当有数据可读时，读取所有数据
    // 读取所有数据并追加到缓冲区
    buffer_.append(socket_.readAll());

    QDataStream stream(&buffer_, QIODevice::ReadOnly);
    stream.setVersion(QDataStream::Qt_5_0);

    forever {
      //先解析头部
      if (!b_recv_pending_) {
        // 检查缓冲区中的数据是否足够解析出一个消息头（消息ID + 消息长度）
        if (buffer_.size() < static_cast<int>(sizeof(quint16) * 2)) {
          return; // 数据不够，等待更多数据
        }

        // 预读取消息ID和消息长度，但不从缓冲区中移除
        stream >> message_id_ >> message_len_;

        //将buffer 中的前四个字节移除
        buffer_ = buffer_.mid(sizeof(quint16) * 2);

        // 输出读取的数据
        qDebug() << "Message ID:" << message_id_ << ", Length:" << message_len_;
      }

      // buffer剩余长读是否满足消息体长度，不满足则退出继续等待接受
      if (buffer_.size() < message_len_) {
        b_recv_pending_ = true;
        return;
      }

      b_recv_pending_ = false;
      // 读取消息体
      QByteArray messageBody = buffer_.mid(0, message_len_);
      qDebug() << "receive body msg is " << messageBody;

      buffer_ = buffer_.mid(message_len_);
    }
  });

  // 处理错误
  QObject::connect(
      &socket_,
      QOverload<QAbstractSocket::SocketError>::of(&QTcpSocket::errorOccurred),
      [&](QAbstractSocket::SocketError socketError) {
        Q_UNUSED(socketError)
        switch (socketError) {
        case QTcpSocket::ConnectionRefusedError:
          qDebug() << "Connection Refused!";
          emit sig_con_success(false);
          break;
        case QTcpSocket::RemoteHostClosedError:
          qDebug() << "Remote Host Closed Connection!";
          break;
        case QTcpSocket::HostNotFoundError:
          qDebug() << "Host Not Found!";
          emit sig_con_success(false);
          break;
        case QTcpSocket::SocketTimeoutError:
          qDebug() << "Connection Timeout!";
          emit sig_con_success(false);
          break;
        case QTcpSocket::NetworkError:
          qDebug() << "Network Error!";
          break;
        default:
          qDebug() << "Other Error!";
          break;
        }
        qDebug() << "Error:" << socket_.errorString();
      });

  // 处理连接断开
  QObject::connect(&socket_, &QTcpSocket::disconnected,
                   [&]() { qDebug() << "Disconnected from server."; });

  QObject::connect(this, &TcpMgr::sig_send_data, this, &TcpMgr::slot_send_data);
}

void TcpMgr::slot_tcp_connect(ServerInfo si) {
  qDebug() << "receive tcp connect signal";
  // 尝试连接到服务器
  qDebug() << "Connecting to server...host" << si.Host << ":" << si.Port;
  host_ = si.Host;
  port_ = static_cast<uint16_t>(si.Port.toUInt());
  socket_.connectToHost(si.Host, port_);
}

void TcpMgr::slot_send_data(ReqId reqId, QString data) {
  uint16_t id = static_cast<int32_t>(reqId);

  // 将字符串转换为UTF-8编码的字节数组
  QByteArray dataBytes = data.toUtf8();

  // 计算长度（使用网络字节序转换）
  quint16 len = static_cast<quint16>(data.size());

  // 创建一个QByteArray用于存储要发送的所有数据
  QByteArray block;
  QDataStream out(&block, QIODevice::WriteOnly);

  // 设置数据流使用网络字节序
  out.setByteOrder(QDataStream::BigEndian);

  // 写入ID和长度
  out << id << len;

  // 添加字符串数据
  block.append(dataBytes);

  // 发送数据
  socket_.write(block);
}

void TcpMgr::initHandlers() {
  //注册获取登录回包逻辑
  handlers_.insert(
      ReqId::ID_CHAT_LOGIN_RSP, [this](ReqId id, int len, QByteArray data) {
        Q_UNUSED(len);
        qDebug() << "handle id is " << static_cast<int32_t>(id) << " data is "
                 << data;
        // 将QByteArray转换为QJsonDocument
        QJsonDocument jsonDoc = QJsonDocument::fromJson(data);
        // 检查转换是否成功
        if (jsonDoc.isNull()) {
          qDebug() << "Failed to create QJsonDocument.";
          return;
        }

        QJsonObject jsonObj = jsonDoc.object();
        qDebug() << "data jsonobj is " << jsonObj;
        if (!jsonObj.contains("error")) {
          int err = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
          qDebug() << "Login Failed, err is Json Parse Err" << err;
          emit sig_login_failed(err);
          return;
        }

        int err = jsonObj["error"].toInt();
        if (err != static_cast<int32_t>(ErrorCodes::NO_ERROR)) {
          qDebug() << "Login Failed, err is " << err;
          emit sig_login_failed(err);
          return;
        }
        UserMgr::getinstance()->SetUid(jsonObj["uid"].toInt());
        UserMgr::getinstance()->SetName(jsonObj["name"].toString());
        UserMgr::getinstance()->SetToken(jsonObj["token"].toString());
        emit sig_swich_chatdlg();
      });
}

void TcpMgr::handle_msg(ReqId id, int len, QByteArray data) {
  auto find_iter = handlers_.find(id);
  if (find_iter == handlers_.end()) {
    qDebug() << "not found id [" << static_cast<int32_t>(id) << "] to handle";
    return;
  }

  find_iter.value()(id, len, data);
}
