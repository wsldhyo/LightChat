#ifndef HTTP_MANAGER_HPP
#define HTTP_MANAGER_HPP
#include "constant.hpp"
#include "singleton.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QString>
#include <QUrl>

/**
 * @brief HTTP请求的管理类, 负责验证客户端界面请求，如果请求无误则转发给服务器，
 * 收到服务器的答复后，将发送信号通知相应对象处理HTTP回复
 * 
 */
class HttpManager : public QObject,
                    public Singleton<HttpManager>,
                    public std::enable_shared_from_this<HttpManager> {
  Q_OBJECT
public:
  ~HttpManager() = default;

private:
  // 构造函数私有，需要将Singleton基类设置为友元
  friend class Singleton<HttpManager>; 
  explicit HttpManager() = default;

  void post_http_request(QUrl _url, QJsonObject _json, RequestID _req_ID,
                         Modules _modules);

  QNetworkAccessManager manager_;

private:
  void create_connection();
private slots:
  void slot_http_finished(QString _res, RequestID _req_ID, Modules _modules,
                          ErrorCode _ec);
signals:
  void sig_http_finished(QString _res, RequestID _req_ID, Modules _modules,
                         ErrorCode _ec);
  void sig_reg_mod_finished(QString _res, RequestID _req_ID, Modules _modules,
                            ErrorCode _ec);
};
#endif