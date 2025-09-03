#ifndef HTTP_CONN_HPP
#define HTTP_CONN_HPP

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <unordered_map>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace asio = boost::asio;   // from <boost/asio.hpp>
using tcp = asio::ip::tcp;      // from <boost/asio/ip/tcp.hpp>

/**
 @brief 管理一个与客户端的http连接，并处理相应的HTTP请求
*/
class HttpConn : public std::enable_shared_from_this<HttpConn> {
  friend class LogicSystem;

public:
  HttpConn(asio::io_context& ioc);
  ~HttpConn();
  void start();
  tcp::socket& get_socket();
private:
  /**
   @brief 开启定时器，定时器到时后，若没有响应则直接关闭连接
  */
  void check_deadline();

  /**
   @brief 发送http响应
  */
  void write_response();

  /**
   @brief 分发http请求给对应的请求处理函数
  */
  void handle_req();

  /**
   @brief 处理Get请求
  */
  void handle_get_req();

  /**
   @brief 处理Post请求
  */
  void handle_post_req();

  /**
   @brief 预处理Get请求的参数
  */
  void pre_parse_get_param();

private:
  tcp::socket socket_;
  beast::flat_buffer buffer_{8192};             // 用来接受数据
  http::request<http::dynamic_body> request_;   // 存储客户端请求
  http::response<http::dynamic_body> response_; // 用于给客户端回包
  asio::steady_timer deadline_{                 // 判断请求是否超时
                               socket_.get_executor(),
                               std::chrono::seconds(60)};

  std::string get_url_;  
  std::unordered_map<std::string, std::string> get_params_;
};
#endif