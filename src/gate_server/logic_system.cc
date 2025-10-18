#include "logic_system.hpp"
#include "httpconn.hpp"
#include "manager/mysql_manager.hpp"
#include "manager/redis_manager.hpp"
#include "rpc_client/status_rpc_client.hpp"
#include "rpc_client/vertify_rpc_client.hpp"
#include "utility/constant.hpp"
#include <iostream>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>

LogicSystem::LogicSystem() {
  register_get_handlers(GET_TEST_URL, [](std::shared_ptr<HttpConn> connection) {
    beast::ostream(connection->response_.body())
        << "receive get_test req " << '\n';
    int i = 0;
    // 简单打印get请求的参数，表示处理成功
    for (auto &elem : connection->get_params_) {
      i++;
      beast::ostream(connection->response_.body())
          << "param" << i << " key is " << elem.first;
      beast::ostream(connection->response_.body())
          << ", "
          << " value is " << elem.second << '\n';
    }
  });

  register_post_handlers(
      POST_GET_VERFIY_CODE, [](std::shared_ptr<HttpConn> connection) {
        LogicSystem::handle_post_get_vertify_code(connection);
      });

  register_post_handlers(POST_REG_USER,
                         [](std::shared_ptr<HttpConn> connection) {
                           LogicSystem::handle_post_register_user(connection);
                         });

  register_post_handlers(POST_RESET_PWD,
                         [](std::shared_ptr<HttpConn> connection) {
                           LogicSystem::handle_post_reset_pwd(connection);
                         });
  register_post_handlers(POST_USER_LOGIN,
                         [](std::shared_ptr<HttpConn> connection) {
                           LogicSystem::handle_post_user_login(connection);
                         });
}

LogicSystem::~LogicSystem() {}

bool LogicSystem::handle_get_req(std::string path,
                                 std::shared_ptr<HttpConn> connection) {
  auto handler = get_handlers_.find(path);
  if (handler == get_handlers_.end()) {
    return false;
  }
  handler->second(connection);
  return true;
}

bool LogicSystem::handle_post_req(std::string path,
                                  std::shared_ptr<HttpConn> connection) {
  std::cout << "Logic Sys handle post req:" << path << '\n';
  auto handler = post_handlers_.find(path);
  if (handler == post_handlers_.end()) {
    return false;
  }
  handler->second(connection);
  return true;
}

void LogicSystem::register_get_handlers(std::string_view url,
                                        HttpHandler_t handler) {
  get_handlers_.insert(make_pair(url, handler));
}

void LogicSystem::register_post_handlers(std::string_view url,
                                         HttpHandler_t handler) {
  post_handlers_.insert(make_pair(url, handler));
}

bool LogicSystem::handle_post_get_vertify_code(
    std::shared_ptr<HttpConn> connection) {
  auto body_str =
      boost::beast::buffers_to_string(connection->request_.body().data());
  std::cout << "receive body is " << body_str << '\n';
  connection->response_.set(http::field::content_type, "text/json");
  Json::Value root;
  Json::Reader reader;
  Json::Value src_root;
  bool parse_success = reader.parse(body_str, src_root);
  if (!parse_success) {
    std::cout << "Failed to parse JSON data!" << '\n';
    root["error"] = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }

  if (!src_root.isMember("email")) {
    std::cout << "Failed to parse JSON data!" << '\n';
    root["error"] = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }

  auto email = src_root["email"].asString();
  GetVertifyRsp rsp = VertifyRPCClient::get_instance()->get_vertify_code(email);
  std::cout << "email is " << email << '\n';
  root["error"] = rsp.error();
  root["email"] = src_root["email"];
  std::string jsonstr = root.toStyledString();
  beast::ostream(connection->response_.body()) << jsonstr;
  return true;
}

bool LogicSystem::handle_post_register_user(
    std::shared_ptr<HttpConn> connection) {
  auto body_str =
      boost::beast::buffers_to_string(connection->request_.body().data());
  std::cout << "receive body is " << body_str << std::endl;
  connection->response_.set(http::field::content_type, "text/json");
  Json::Value root;
  Json::Reader reader;
  Json::Value src_root;
  bool parse_success = reader.parse(body_str, src_root);
  if (!parse_success) {
    std::cout << "Failed to parse JSON data!" << std::endl;
    root["error"] = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }

  auto email = src_root["email"].asString();
  auto name = src_root["user"].asString();
  auto pwd = src_root["pwd"].asString();
  auto confirm = src_root["confirm"].asString();
  // 验证两次密码输入是否一致，客户端和服务端都需要验证。客户端验证主要是过滤无效输入，真正决定的是服务端
  if (pwd != confirm) {
    std::cout << "password err. pwd:" << pwd << "confirm pwd:" << confirm
              << std::endl;
    root["error"] = static_cast<int32_t>(ErrorCodes::CONFIRM_PWD_DISMATCH);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }

  // 验证验证是否匹配
  std::string vertify_code;
  bool b_get_vertify = RedisMgr::get_instance()->get(
      std::string(REDIS_VERTIFY_CODE_PREFIX) + src_root["email"].asString(),
      vertify_code);
  if (!b_get_vertify) {
    std::cout << " get vertify code expired" << std::endl;
    root["error"] = static_cast<int32_t>(ErrorCodes::VERTIFY_CODE_EXPIRED);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }

  // 验证验证码是否过期
  if (vertify_code != src_root["vertifycode"].asString()) {
    std::cout << " vertify code error" << std::endl;
    root["error"] = static_cast<int32_t>(ErrorCodes::VERTIFY_CODE_DISMATCH);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }

  //查找数据库判断用户是否存在
  int uid = MysqlMgr::get_instance()->reg_user(name, email, pwd);
  if (uid == 0 || uid == -1) {
    std::cout << " user or email exist" << std::endl;
    root["error"] = static_cast<int32_t>(ErrorCodes::REG_USER_EXISTS);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }
  root["error"] = 0;
  root["uid"] = uid;
  root["email"] = email;
  root["user"] = name;
  root["pwd"] = pwd;
  root["confirm"] = confirm;
  root["vertifycode"] = src_root["vertifycode"].asString();
  std::string jsonstr = root.toStyledString();
  beast::ostream(connection->response_.body()) << jsonstr;
  return true;
}

//重置密码回调逻辑
bool LogicSystem::handle_post_reset_pwd(std::shared_ptr<HttpConn> connection) {
  auto body_str =
      boost::beast::buffers_to_string(connection->request_.body().data());
  std::cout << "receive body is " << body_str << std::endl;
  connection->response_.set(http::field::content_type, "text/json");
  Json::Value root;
  Json::Reader reader;
  Json::Value src_root;
  bool parse_success = reader.parse(body_str, src_root);
  if (!parse_success) {
    std::cout << "Failed to parse JSON data!" << std::endl;
    root["error"] = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }

  auto email = src_root["email"].asString();
  auto name = src_root["user"].asString();
  auto pwd = src_root["passwd"].asString();

  //先查找redis中email对应的验证码是否合理
  std::string vertify_code;
  bool b_get_vertify = RedisMgr::get_instance()->get(
      std::string(REDIS_VERTIFY_CODE_PREFIX) + src_root["email"].asString(),
      vertify_code);
  if (!b_get_vertify) {
    std::cout << " get vertify code expired" << std::endl;
    root["error"] = static_cast<int32_t>(ErrorCodes::VERTIFY_CODE_EXPIRED);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }

  if (vertify_code != src_root["vertifycode"].asString()) {
    std::cout << " vertify code error" << std::endl;
    root["error"] = static_cast<int32_t>(ErrorCodes::VERTIFY_CODE_DISMATCH);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }
  //查询数据库判断用户名和邮箱是否匹配
  bool email_valid = MysqlMgr::get_instance()->check_email(name, email);
  if (!email_valid) {
    std::cout << " user email not match" << std::endl;
    root["error"] = static_cast<int32_t>(ErrorCodes::EMAIL_DISMATCH);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }

  //更新密码为最新密码
  std::cout << "reset new pwd:" << pwd << '\n';
  bool b_up = MysqlMgr::get_instance()->update_pwd(name, pwd);
  if (!b_up) {
    std::cout << " update pwd failed" << std::endl;
    root["error"] = static_cast<int32_t>(ErrorCodes::PWD_UPDATE_FAILED);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }

  std::cout << "succeed to update password" << pwd << std::endl;
  root["error"] = 0;
  root["email"] = email;
  root["user"] = name;
  root["passwd"] = pwd;
  root["vertifycode"] = src_root["vertifycode"].asString();
  std::string jsonstr = root.toStyledString();
  beast::ostream(connection->response_.body()) << jsonstr;
  return true;
}

bool LogicSystem::handle_post_user_login(std::shared_ptr<HttpConn> connection) {
  auto body_str =
      boost::beast::buffers_to_string(connection->request_.body().data());
  std::cout << "handle user login "
            << "receive body is " << body_str << std::endl;
  connection->response_.set(http::field::content_type, "text/json");
  Json::Value root;
  Json::Reader reader;
  Json::Value src_root;
  bool parse_success = reader.parse(body_str, src_root);
  if (!parse_success) {
    std::cout << "Failed to parse JSON data!" << std::endl;
    root["error"] = static_cast<int32_t>(ErrorCodes::PARSE_JSON_FAILED);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }

  auto email = src_root["email"].asString();
  auto pwd = src_root["passwd"].asString();
  UserInfo userInfo;
  //查询数据库判断用户名和密码是否匹配
  bool pwd_valid = MysqlMgr::get_instance()->check_pwd(email, pwd, userInfo);
  if (!pwd_valid) {
    std::cout << " user pwd not match" << std::endl;
    root["error"] = static_cast<int32_t>(ErrorCodes::PWD_INCORRECT);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }

  //查询StatusServer找到合适的连接
  std::cout << "search chat server\n";
  auto reply = StatusGrpcClient::get_instance()->get_chatserver(userInfo.uid);
  if (reply.error()) {
    std::cout << " grpc get chat server failed, error is " << reply.error()
              << std::endl;
    root["error"] = static_cast<int32_t>(ErrorCodes::RPC_CALL_FAILED);
    std::string jsonstr = root.toStyledString();
    beast::ostream(connection->response_.body()) << jsonstr;
    return false;
  }
  std::cout << "chat server:" << reply.host() << ":" << reply.port() << '\n';
  std::cout << "succeed to load userinfo uid is " << userInfo.uid << std::endl;
  root["error"] = 0;
  root["email"] = email;
  root["uid"] = userInfo.uid;
  root["token"] = reply.token();
  root["host"] = reply.host();
  root["port"] = reply.port();
  std::string jsonstr = root.toStyledString();
  beast::ostream(connection->response_.body()) << jsonstr;
  return true;
}