#ifndef LOGIC_SYSTEM_HPP
#define LOGIC_SYSTEM_HPP
#include "utility/singleton.hpp"
#include <functional>
#include <map>
#include <memory>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <boost/beast/http.hpp>
#include <memory>
#include <string_view>

namespace beast = boost::beast; // from <boost/beast.hpp>
namespace http = beast::http;   // from <boost/beast/http.hpp>
namespace asio = boost::asio;   // from <boost/asio.hpp>
using tcp = asio::ip::tcp;      // from <boost/asio/ip/tcp.hpp>

class HttpConn;
using HttpHandler_t =
    std::function<void(std::shared_ptr<HttpConn>)>; // Http回调函数类型

class LogicSystem : public Singleton<LogicSystem> {
  friend class Singleton<LogicSystem>;

public:
  ~LogicSystem();
  /**
   @brief 调用回调集中的回调处理Get请求
   @param url Get请求的url，用于查找回调函数
   @param connection 要处理的Http连接
   @return bool
          true 表示处理成功
          false 没有找到相应的处理函数
  */
  bool handle_get_req(std::string path, std::shared_ptr<HttpConn> connection);

  /**
   @brief 调用回调集中的回调处理Post请求
   @param url Post请求的url，用于查找回调函数
   @param connection 要处理的Http连接
   @return bool
          true 表示处理成功
          false 没有找到相应的处理函数
  */
  bool handle_post_req(std::string path, std::shared_ptr<HttpConn> connection);

  /**
      @brief 注册处理Get请求的回调函数
      @param url Get请求的url
      @param handler 处理Get请求的回调函数
      @return void
  */
  void register_get_handlers(std::string_view url, HttpHandler_t handler);

  /**
      @brief 注册处理Post请求的回调函数
      @param url Post请求的url
      @param handler 处理Post请求的回调函数
      @return void
  */
  void register_post_handlers(std::string_view url, HttpHandler_t handler);

  // 处理获取验证码的Post请求
  static bool
  handle_post_get_vertify_code(std::shared_ptr<HttpConn> connection);

  // 处理获取验证码的Post请求
  static bool handle_post_register_user(std::shared_ptr<HttpConn> connection);

  // 处理重置密码的Post请求
  static bool handle_post_reset_pwd(std::shared_ptr<HttpConn> connection);

  // 处理用户登录的Post请求
  static bool handle_post_user_login(std::shared_ptr<HttpConn> connection);

private:
  LogicSystem();
  std::map<std::string_view, HttpHandler_t>
      post_handlers_; // 处理Post请求的回调集
  std::map<std::string_view, HttpHandler_t>
      get_handlers_; // 处理Get请求的回调集
};
#endif