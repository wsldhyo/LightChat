#include "http_connection.hpp"
#include "../../common/toolfunc.hpp"
#include "logic_system.hpp"
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/http/write.hpp>
#include <iostream>
#include <ostream>
#include <string_view>
HttpConnection::HttpConnection(asio::io_context& _ioc)
    : socket_(_ioc) {}

void HttpConnection::start() {
  auto self = shared_from_this();
  http::async_read(socket_, buffer_, request_,
                   [self](beast::error_code _ec, std::size_t bytes_tranferred) {
                     try {
                       if (_ec) {
                         std::cout << "http read err is " << _ec.message()
                                   << std::endl;
                         return;
                       }
                       boost::ignore_unused(bytes_tranferred);
                       self->handle_request();
                       self->check_deadline();

                     } catch (std::exception const &e) {
                       std::cout << "exception occured when http reading: "
                                 << e.what() << std::endl;
                     }
                   });
}

void HttpConnection::handle_request() {
  // 响应版本与请求版本一致
  response_.version(request_.version());
  // 短连接
  response_.keep_alive(false);
  if (request_.method() == http::verb::get) {
    parse_get_params();
    bool success = LogicSystem::get_instance()->handle_get_request(
        get_url_, shared_from_this());
    if (!success) {
      // 构造失败结果
      // 404错误
      response_.result(http::status::not_found);
      // 回应类型
      response_.set(http::field::content_type, "text/plain");
      // 回应消息体
      beast::ostream(response_.body())
          << "url not found: " << request_.target();
      write_response();
      return;
    }
  }
  else if (request_.method() == http::verb::post) {
    bool success = LogicSystem::get_instance()->handle_post_request(
        request_.target(), shared_from_this());
    if (!success) {
      // 构造失败结果
      // 404错误
      response_.result(http::status::not_found);
      // 回应类型
      response_.set(http::field::content_type, "text/plain");
      // 回应消息体
      beast::ostream(response_.body())
          << "url not found: " << request_.target();
      write_response();
      return;
    }
    else {
    // TODO 不支持的请求
    }
    response_.result(http::status::ok);
    response_.set(http::field::server, "GateServer");
    // 回应消息体
    write_response();
    return;
  }
}

void HttpConnection::write_response() {
  auto self = shared_from_this();
  response_.content_length(response_.body().size());
  http::async_write(
      socket_, response_,
      [self](beast::error_code _ec, std::size_t bytes_tranferred) {
        // http短连接，关闭服务器发送端
        self->socket_.shutdown(tcp::socket::shutdown_send);
        // 已经成功响应，取消超时检查
        self->deadline_timer.cancel();
      });
}

void HttpConnection::check_deadline() {
  auto self = shared_from_this();
  deadline_timer.async_wait([self](beast::error_code _ec) {
    if (!_ec) {
      // 没有错误发生，但超时，关闭socket连接
      // 注意这会强制导致客户端的连接断开，容易导致time
      // wait，实际开发中服务器不要主动关闭客户端连接
      self->socket_.close(_ec);
    }
  });
}
void HttpConnection::parse_get_params() {
  // 提取 路由URI
  auto uri = request_.target();
  // 查找查询字符串的开始位置（即 '?' 的位置）
  auto query_pos = uri.find('?');
  if (query_pos == std::string::npos) {
    get_url_ = uri;
    return;
  }
  std::cout << "uri:" << uri << std::endl;
  // 解析？后面的get请求参数
  get_url_ = uri.substr(0, query_pos);
  std::string_view query_string = uri.substr(query_pos + 1);
  std::string key;
  std::string value;
  size_t pos = 0;
  // 查找&分隔的参数键值对，并进行解码操作
  while ((pos = query_string.find('&')) != std::string_view::npos) {
    key.clear();
    value.clear();    
    auto pair = query_string.substr(0, pos);
    size_t eq_pos = pair.find('=');
    if (eq_pos != std::string_view::npos) {

      url_decode(pair.substr(0, eq_pos),
                 key); // 假设有 url_decode 函数来处理URL解码
      url_decode(pair.substr(eq_pos + 1), value);
      get_params_[key] = value;
    }
    query_string.remove_prefix(pos + 1); // 移除已解析部分
  }
  // 处理最后一个参数对（如果没有 & 分隔符）
  if (!query_string.empty()) {
    size_t eq_pos = query_string.find('=');
    if (eq_pos != std::string_view::npos) {
      key.clear();
      value.clear();
      url_decode(query_string.substr(0, eq_pos), key);
      url_decode(query_string.substr(eq_pos + 1), value);
      get_params_[key] = value;
    }
  }
}
  tcp::socket& HttpConnection::get_socket(){
    return socket_;
  }