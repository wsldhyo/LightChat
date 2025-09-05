#ifndef HTTP_MANAGER_HPP
#define HTTP_MANAGER_HPP
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <QUrl>
#include <memory>

#include "utility/constant.hpp"
#include "utility/singleton.hpp"

/**
 @brief 为客户端管理http发送接收等请求
*/
class HttpMgr : public QObject,
                public Singleton<HttpMgr>,
                public std::enable_shared_from_this<HttpMgr> {
  Q_OBJECT
public:
  ~HttpMgr();
  void post_http_req(QUrl url, QJsonObject json, ReqId req_id, Modules mod);

private:
  friend class Singleton<HttpMgr>;
  HttpMgr();
  void create_connection();
private slots:
  // 处理http请求的完成的槽函数
  void slot_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);

signals:
  // http请求完成的信号（请求已经响应）
  void sig_http_finish(ReqId id, QString res, ErrorCodes err, Modules mod);
  // 通知注册模块的http请求已经响应
  void sig_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
  // 通知重置密码模块的http请求已经响应
  void sig_reset_mod_finish(ReqId id, QString res, ErrorCodes err);
  // 通知登录模块的http请求已经响应
  void sig_login_mod_finish(ReqId id, QString res, ErrorCodes err);

private:
  QNetworkAccessManager _manager;
};
#endif