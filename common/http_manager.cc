
#include "http_manager.hpp"
#include "constant.hpp"
#include <QNetworkReply>
#include <qdebug.h>
#include <qobjectdefs.h>

  HttpManager:: HttpManager(){
    create_connection();
  }
// 投递Http请求
void HttpManager::post_http_request(QUrl _url, QJsonObject _json,
                                    RequestID _req_ID, Modules _modules) {
  QByteArray data = QJsonDocument(_json).toJson();

  // 构造请求
  QNetworkRequest request(_url);
  // 路由
  request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
  // 包体长度
  request.setHeader(QNetworkRequest::ContentLengthHeader,
                    QByteArray::number(data.length()));
  auto self = shared_from_this();

  // 投递Http请求
  QNetworkReply *reply = manager_.post(request, data);
  // 事件循环中，QNetworkAccessManager 的线程中，槽函数被触发 
  connect(reply, &QNetworkReply::finished, [reply, self, _req_ID, _modules]() {
    // 回调判断Http回复，并发送信号通知收到回复
    if (reply->error() != QNetworkReply::NoError) {
      qDebug() << reply->errorString();
      // 发信号通知完成
      emit self->sig_http_finished("", _req_ID, _modules,
                                   ErrorCode::NETWORK_ERROR);
      reply->deleteLater(); // 告诉Qt，replay无用时回收
      return;
    }
    // 无错误处理
    QString res = reply->readAll();
    qDebug() << "request res:" << res;
    emit self->sig_http_finished(res, _req_ID, _modules, ErrorCode::NO_ERROR);
    reply->deleteLater(); // 告诉Qt，replay无用时回收
    return;
  });
}

void HttpManager::create_connection() {
  connect(this, &HttpManager::sig_http_finished,
          &HttpManager::slot_http_finished);
}


void HttpManager::slot_http_finished(QString _res, RequestID _req_ID,
                                     Modules _modules, ErrorCode _ec) {
  qDebug() << "slot http finished mod:" << static_cast<int>(Modules::REGISTER_MOD);
  if (_modules == Modules::REGISTER_MOD) {
    emit sig_reg_mod_finished(_res, _req_ID, _modules,  _ec);
  }
}