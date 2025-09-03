
#include "httpconn.hpp"
#include "logic_system.hpp"
#include "utility/toolfunc.hpp"
#include <iostream>

HttpConn::HttpConn(asio::io_context &ioc) : socket_(ioc) {}

HttpConn::~HttpConn() {}

void HttpConn::start() {
  auto self = shared_from_this();
  http::async_read(socket_, buffer_, request_,
                   [self](beast::error_code ec, std::size_t bytes_transferred) {
                     try {
                       if (ec) {
                         std::cout << "http read err is " << ec.what() << '\n';
                         return;
                       }
                       //处理读到的数据
                       boost::ignore_unused(bytes_transferred);
                       self->handle_req();
                       self->check_deadline();
                     } catch (std::exception &exp) {
                       std::cout << "exception is " << exp.what() << '\n';
                     }
                   });
}

tcp::socket &HttpConn::get_socket() { return socket_; }

void HttpConn::handle_req() {
  //设置Http版本
  response_.version(request_.version());
  //设置为Http短链接
  response_.keep_alive(false);
  // 处理Get请求
  if (request_.method() == http::verb::get) {
    handle_get_req();
  } else if (request_.method() == http::verb::post) {
    handle_post_req();
  }
}

void HttpConn::handle_get_req() {
  pre_parse_get_param();
  bool success =
      LogicSystem::getinstance()->handle_get_req(get_url_, shared_from_this());
  if (!success) {
    // 请求处理失败，设置错误的回包信息
    response_.result(http::status::not_found);
    response_.set(http::field::content_type, "text/plain");
    beast::ostream(response_.body()) << "url not found\r\n";
  } else {
    // 请求处理成功，设置成功的回包信息
    response_.result(http::status::ok);
    response_.set(http::field::server, "GateServer"); // 告知处理的服务器
  }
  // 发送回包
  write_response();
  return;
}

void HttpConn::handle_post_req() {
  // POST 请求的数据放在 请求体（body), 无需调用pre_parse_get_param解析URL
  // LogicSystem直接通过jsoncpp解析请求体中的数据
  // 逻辑类似handle_get_req，只是调用LogicSystem的handle_post_req函数
  bool success = LogicSystem::getinstance()->handle_post_req(
      request_.target(), shared_from_this());
  if (!success) {
    response_.result(http::status::not_found);
    response_.set(http::field::content_type, "text/plain");
    beast::ostream(response_.body()) << "url not found\r\n";
  } else {
    response_.result(http::status::ok);
    response_.set(http::field::server, "GateServer");
  }
  write_response();
  return;
}

void HttpConn::write_response() {
  auto self = shared_from_this();
  response_.content_length(response_.body().size());
  http::async_write(
      socket_, response_,
      [self](beast::error_code ec, std::size_t bytes_transferred) {
        // 短连接，发送完毕后关闭连接
        // *****关闭发送端连接即可，后续不再发送
        self->socket_.shutdown(tcp::socket::shutdown_send, ec);
        // 取消超时关闭连接
        self->deadline_.cancel();
      });
}

void HttpConn::check_deadline() {
  auto self = shared_from_this();
  deadline_.async_wait([self](beast::error_code ec) {
    // ec = 0表示正常超时, ec!=0可能是取消定时、或重新设置定时器等
    if (!ec) {
      // ****注意，这里调用clise会关闭双方的连接，主动关闭客户端连接可能引起time
      // wait状态问题
      self->socket_.close();
    }
  });
}

void HttpConn::pre_parse_get_param() {
  // 提取 URI
  auto uri = request_.target();
  // 查找查询字符串的开始位置（即 '?' 的位置）
  auto query_pos = uri.find('?');
  if (query_pos == std::string::npos) {
    get_url_ = uri;
    return; // 没有?，说明没有参数需要解析，直接返回
  }
  get_url_ = uri.substr(0, query_pos);                  // url部分
  std::string query_string = uri.substr(query_pos + 1); // 参数部分
  std::string key;
  std::string value;
  size_t pos = 0;
  // ？后面跟着键值对参数，多个参数使用& 分割，据此提取参数存储到map中
  while ((pos = query_string.find('&')) != std::string::npos) {
    auto pair = query_string.substr(0, pos);
    size_t eq_pos = pair.find('=');
    if (eq_pos != std::string::npos) {
      url_decode(pair.substr(0, eq_pos), key);
      url_decode(pair.substr(eq_pos + 1), value);
      get_params_[key] = value;
    }
    query_string.erase(0, pos + 1);
  }
  // 处理最后一个参数对，没有&间隔
  // 例如：http://127.0.0.1:1234/?key=value，这里只有一对参数，没有&间隔
  // 或者 http://127.0.0.1:1234/?key=value&key2=value2，最后一个参数没有&
  if (!query_string.empty()) {
    size_t eq_pos = query_string.find('=');
    if (eq_pos != std::string::npos) {
      url_decode(query_string.substr(0, eq_pos), key);
      url_decode(query_string.substr(eq_pos + 1), value);
      get_params_[key] = value;
    }
  }
}