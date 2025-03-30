#ifndef HTTP_CONNECTION_HPP
#define HTTP_CONNECTION_HPP

#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/dynamic_body_fwd.hpp>
#include <boost/beast/http/message_fwd.hpp>
#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>
namespace beast = boost::beast;
namespace http = beast::http;
namespace asio = boost::asio;
class LogicSystem;
using tcp = asio::ip::tcp;
class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
public:
  HttpConnection(asio::io_context& _ioc);
  void start();
  tcp::socket& get_socket();
private:
  friend class LogicSystem;
  void check_deadline();
  void write_response();
  void handle_request();
  
  /**
 * @brief 解析 HTTP GET 请求的查询参数，并存入 get_params_
 * 
 * @details
 * - 解析请求目标 URI，提取 `?` 之前的路由路径 `get_url_`
 * - 解析 `?` 之后的查询参数（`key=value` 格式）
 * - 使用 `&` 分隔多个参数
 * - 解析 `key` 和 `value`，进行 URL 解码后存入 `get_params_`
 * 
 * @note 该函数假设 `url_decode` 可正确解码 URL 编码的参数
 * 
 * @example
 *  请求: `/search?q=hello%20world&lang=en`
 *  解析后:
 *  ```
 *  get_url_ = "/search"
 *  get_params_ = { "q" : "hello world", "lang" : "en" }
 *  ```
 */
  void parse_get_params();
  tcp::socket socket_;
  beast::flat_buffer buffer_{8192};
  http::request<http::dynamic_body> request_;
  http::response<http::dynamic_body> response_;
  boost::asio::steady_timer deadline_timer{
      socket_.get_executor(), std::chrono::seconds(60) /*超时时间*/};
  
  std::string get_url_;
  std::unordered_map<std::string, std::string> get_params_;
};

#endif
