#include "tcp_manager.hpp"
#include "user_data.hpp"
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
        qDebug() << "TCP Message ID:" << message_id_
                 << ", Length:" << message_len_;
      }

      // buffer剩余长读是否满足消息体长度，不满足则退出继续等待接受
      if (buffer_.size() < message_len_) {
        b_recv_pending_ = true;
        return;
      }

      b_recv_pending_ = false;
      // 读取消息体
      QByteArray messageBody = buffer_.mid(0, message_len_);
      // qDebug() << "receive body msg is " << messageBody;

      buffer_ = buffer_.mid(message_len_);
      handle_msg(static_cast<ReqId>(message_id_), message_len_, messageBody);
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
  initHandlers();
}

void TcpMgr::slot_tcp_connect(ServerInfo si) {
  qDebug() << "receive tcp connect signal";
  // 尝试连接到服务器
  qDebug() << "Connecting to server...host" << si.Host << ":" << si.Port;
  host_ = si.Host;
  port_ = static_cast<uint16_t>(si.Port.toUInt());
  socket_.connectToHost(si.Host, port_);
}

void TcpMgr::slot_send_data(ReqId reqId, QByteArray dataBytes) {
  uint16_t id = static_cast<int32_t>(reqId);

  // 计算长度（使用网络字节序转换）
  quint16 len = static_cast<quint16>(dataBytes.size());

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
  handlers_.insert(ReqId::ID_CHAT_LOGIN_RSP, [this](ReqId id, int len,
                                                    QByteArray data) {
    Q_UNUSED(len);
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
      qDebug() << "Json parse: Missing errro filed" << err;
      // TODO 换成通用的信号，在各个回调中使用
      emit sig_login_failed(err);
      return;
    }

    int err = jsonObj["error"].toInt();
    if (err != static_cast<int32_t>(ErrorCodes::NO_ERROR)) {
      qDebug() << "Login Failed, err is " << err;
      emit sig_login_failed(err);
      return;
    }
    // 设置服务器返回的用户的基本信息
    UserMgr::getinstance()->set_user_info(std::make_shared<UserInfo>(
        jsonObj["uid"].toInt(), jsonObj["name"].toString(),
        jsonObj["nick"].toString(), jsonObj["icon"].toString(),
        jsonObj["sex"].toInt()));
    // 尝试获取好友申请列表（可能包好离线时的申请）
    if (jsonObj.contains("apply_list")) { //如果服务器返回了好友申请列表，则添加
      UserMgr::getinstance()->append_apply_list(
          jsonObj["apply_list"].toArray());
    }

    if (jsonObj.contains(
            "friend_list")) { //如果服务器返回了好友申请列表，则添加
      UserMgr::getinstance()->append_friend_list(
          jsonObj["friend_list"].toArray());
    }
    emit sig_switch_chatdlg();
    // qDebug() << "switch chat dlg";
  });

  // 处理服务器搜索用户请求的响应
  handlers_.insert(
      ReqId::ID_SEARCH_USER_RSP, [this](ReqId id, int len, QByteArray data) {
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
        // 检查Json解析是否成功
        if (!jsonObj.contains("error")) {
          int err = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
          qDebug() << "Query user failed, Missing error filed" << err;
          // TODO 换成通用的信号，在各个回调中使用
          emit sig_user_search(nullptr);
          return;
        }
        // 检查服务器回包的错误码
        int err = jsonObj["error"].toInt();
        if (err != static_cast<int32_t>(ErrorCodes::NO_ERROR)) {
          qDebug() << "Query user failed, err is " << err;
          emit sig_user_search(nullptr);
          return;
        }
        // 解析出搜索到的用户信息
        auto search_info = std::make_shared<SearchInfo>(
            jsonObj["uid"].toInt(), jsonObj["name"].toString(),
            jsonObj["nick"].toString(), jsonObj["desc"].toString(),
            jsonObj["sex"].toInt(), jsonObj["icon"].toString());
        // 将搜索结果发送给聊天界面
        emit sig_user_search(search_info);
      });
  // 处理服务器对申请好友的回包
  handlers_.insert(ReqId::ID_NOTIFY_FRIEND_APPLY_REQ, [this](ReqId id, int len,
                                                             QByteArray data) {
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
    // 检查Json解析是否成功
    if (!jsonObj.contains("error")) {
      int err = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
      qDebug() << "Apply friend, Missing error filed" << err;
      return;
    }
    // 检查服务器回包的错误码
    int err = jsonObj["error"].toInt();
    if (err != static_cast<int32_t>(ErrorCodes::NO_ERROR)) {
      qDebug() << "Apply friend, err is " << err;
      return;
    }
    qDebug() << "Receive apply friend request.";
    // 解析出申请信息，并发送信号，让客户端显示好友申请，待用户处理申请
    int from_uid = jsonObj["applyuid"].toInt();
    QString name = jsonObj["name"].toString();
    QString desc = jsonObj["desc"].toString();
    QString icon = jsonObj["icon"].toString();
    QString nick = jsonObj["nick"].toString();
    int sex = jsonObj["sex"].toInt();

    auto apply_info =
        std::make_shared<AddFriendApply>(from_uid, name, desc, icon, nick, sex);
    // 通知好友申请界面，展示好友申请消息
    emit sig_recv_friend_apply(apply_info);
  });
  // 处理服务器处理申请好友请求后的回包（可能成功通知对方有好友申请，也可能有错误而没有通知对方）
  handlers_.insert(
      ReqId::ID_APPLY_FRIEND_RSP, [this](ReqId id, int len, QByteArray data) {
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

        if (!jsonObj.contains("error")) {
          auto err = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
          qDebug() << "Add Friend Failed, err is Json Parse Err" << err;
          return;
        }

        int err = jsonObj["error"].toInt();
        if (err != static_cast<int32_t>(ErrorCodes::NO_ERROR)) {
          qDebug() << "Add Friend Failed, err is " << err;
          return;
        }

        qDebug() << "Add Friend Success ";
      });
  // 服务器响应好友认证请求后的处理
  handlers_.insert(
      ReqId::ID_AUTH_FRIEND_RSP, [this](ReqId id, int len, QByteArray data) {
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

        if (!jsonObj.contains("error")) {
          auto err = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
          qDebug() << "Add Friend Failed, err is Json Parse Err" << err;
          return;
        }

        int err = jsonObj["error"].toInt();
        if (err != static_cast<int32_t>(ErrorCodes::NO_ERROR)) {
          qDebug() << "Add Friend Failed, err is " << err;
          return;
        }

        qDebug() << "Add Friend Success ";
        auto name = jsonObj["name"].toString();
        auto nick = jsonObj["nick"].toString();
        auto icon = jsonObj["icon"].toString();
        auto sex = jsonObj["sex"].toInt();
        auto uid = jsonObj["uid"].toInt();
        auto auth_rsp = std::make_shared<AuthRsp>(uid, name, nick, icon, sex);
        emit sig_friend_apply_rsp(auth_rsp);
      });
  // 收到他人好友认证请求后的处理
  handlers_.insert(ReqId::ID_NOTIFY_AUTH_FRIEND_REQ, [this](ReqId id, int len,
                                                            QByteArray data) {
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
    if (!jsonObj.contains("error")) {
      auto err = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
      qDebug() << "Auth Friend Failed, err is " << err;
      return;
    }

    int err = jsonObj["error"].toInt();
    if (err != static_cast<int32_t>(ErrorCodes::NO_ERROR)) {
      qDebug() << "Auth Friend Failed, err is " << err;
      return;
    }

    int from_uid = jsonObj["fromuid"].toInt();
    QString name = jsonObj["name"].toString();
    QString nick = jsonObj["nick"].toString();
    QString icon = jsonObj["icon"].toString();
    int sex = jsonObj["sex"].toInt();

    auto auth_info =
        std::make_shared<AuthInfo>(from_uid, name, nick, icon, sex);

    emit sig_recv_friend_auth(auth_info);
  });

  // 服务器响应发送消息给对方
  handlers_.insert(
      ReqId::ID_TEXT_CHAT_MSG_RSP, [this](ReqId id, int len, QByteArray data) {
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

        if (!jsonObj.contains("error")) {
          int err = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
          qDebug() << "Chat Msg Rsp Failed, Json obj missing \"error\" field"
                   << err;
          return;
        }

        int err = jsonObj["error"].toInt();
        if (err != static_cast<int32_t>(ErrorCodes::NO_ERROR)) {
          qDebug() << "Chat Msg Rsp Failed, err is " << err;
          return;
        }

        qDebug() << "Receive Text Chat Rsp Success ";
        // ui设置送达等标记 todo...
        // TODO 发送失败可以显示红色感叹号，重发按钮等
      });

  // 收到他人消息
  handlers_.insert(ReqId::ID_NOTIFY_TEXT_CHAT_MSG_REQ, [this](ReqId id, int len,
                                                              QByteArray data) {
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

    if (!jsonObj.contains("error")) {
      int err = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
      qDebug() << "Notify Chat Msg Failed, json obj missing \"error\" field"
               << err;
      return;
    }

    int err = jsonObj["error"].toInt();
    if (err != static_cast<int32_t>(ErrorCodes::NO_ERROR)) {
      qDebug() << "Notify Chat Msg Failed, err is " << err;
      return;
    }

    qDebug() << "Receive Text Chat Notify Success ";
    // 拼接为消息对象
    auto msg_ptr = std::make_shared<TextChatMsg>(
        jsonObj["fromuid"].toInt(), jsonObj["touid"].toInt(),
        jsonObj["text_array"].toArray());
    emit sig_recv_text_msg(msg_ptr);
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
