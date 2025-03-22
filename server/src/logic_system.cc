#include "logic_system.hpp"
#include "../../common/constant.hpp"
#include "grpc_vertify_code_client.hpp"
#include "http_connection.hpp"
#include "mysql_manager.hpp"
#include "redis_connection_manager.hpp"
#include <boost/beast/core/buffers_to_string.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/http/field.hpp>
#include <iostream>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>
#include <memory>
#include <ostream>
#include <string>

LogicSystem::LogicSystem() {
  // 处理get请求的回调函数
  register_get_handler("/get_test",
                       [](std::shared_ptr<HttpConnection> _connection) {
                         beast::ostream(_connection->response_.body())
                             << "receive get_test request\n";
                         int i{0};
                         // 打印_connection中存储的get_url中的参数
                         for (auto &element : _connection->get_params_) {
                           ++i;
                           beast::ostream(_connection->response_.body())
                               << "param:" << i << " key is " << element.first;
                           beast::ostream(_connection->response_.body())
                               << " value is " << element.second << "\n";
                         }
                       });

  // 处理获取验证码请求的回调函数
  register_post_handler(
      "/get_vertifycode", [](std::shared_ptr<HttpConnection> _connection) {
        auto body_str =
            beast::buffers_to_string(_connection->request_.body().data());
        std::cout << "receive get_vertifycode body is " << body_str
                  << std::endl;
        _connection->response_.set(http::field::content_type, "text/json");
        // 解析Json格式的请求报文
        Json::Value res_root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        // JSON解析失败
        if (!parse_success) {
          std::cout << "Failed to parse JSON" << std::endl;
          res_root["error"] = static_cast<int>(ErrorCode::JSON_PARSE_FAILED);
          std::string res_json_str = res_root.toStyledString();
          beast::ostream(_connection->response_.body()) << res_json_str;
          return false;
        }
        // 判断Json中是否有key存在
        if (!src_root.isMember("email")) {
          std::cout << "Failed to parse json data, key is not exist"
                    << std::endl;
          res_root["error"] = static_cast<int>(ErrorCode::JSON_PARSE_FAILED);
          std::string res_json_str = res_root.toStyledString();
          beast::ostream(_connection->response_.body()) << res_json_str;
          return false;
        }

        // 通过RPC，向GetVertifycodeServer发起获取验证码服务
        auto email = src_root["email"].asString();
        GetVertifyRsp vertify_reponse =
            GrpcVertifyCodeClient::get_instance()->get_vertify_code(email);
        std::cout << "email is " << email << std::endl;
        res_root["error"] = vertify_reponse.error();
        res_root["email"] = email;
        beast::ostream(_connection->response_.body())
            << res_root.toStyledString();
        return true;
      });

  // 处理用户注册请求的回调函数
  register_post_handler(
      "/usr_register", [](std::shared_ptr<HttpConnection> _connection) {
        auto body_str = boost::beast::buffers_to_string(
            _connection->request_.body().data());
        std::cout << "receive body is " << body_str << std::endl;
        _connection->response_.set(http::field::content_type, "text/json");
        // 解析Json格式的用户注册请求
        Json::Value root;
        Json::Reader reader;
        Json::Value src_root;
        bool parse_success = reader.parse(body_str, src_root);
        if (!parse_success) {
          // 解析失败则返回
          std::cout << "Failed to parse JSON data!" << std::endl;
          root["error"] = static_cast<int>(ErrorCode::JSON_PARSE_FAILED);
          std::string jsonstr = root.toStyledString();
          beast::ostream(_connection->response_.body()) << jsonstr;
          return true;
        }
        // 先查找redis中email对应的验证码是否过期
        std::string vertify_code;
        std::string redis_vertify_code_key =
            CODE_PREFIX + src_root["email"].asString();
        std::cout << "redis vertify code key is " << redis_vertify_code_key
                  << std::endl;
        bool b_get_varify = RedisConnectionManager::get_instance()->get(
            redis_vertify_code_key, vertify_code);
        if (!b_get_varify) {
          std::cout << " get vertify code expired" << std::endl;
          root["error"] = static_cast<int>(ErrorCode::VERTIFY_CODE_EXPIRED);
          std::string jsonstr = root.toStyledString();
          beast::ostream(_connection->response_.body()) << jsonstr;
          return true;
        }
        // 检查客户端输入的验证码和服务端的验证码是否一致
        if (vertify_code != src_root["vertifycode"].asString()) {
          std::cout << " vertify code error" << std::endl;
          root["error"] = static_cast<int>(ErrorCode::VERTIFY_CODE_DISMATCH);
          std::string jsonstr = root.toStyledString();
          beast::ostream(_connection->response_.body()) << jsonstr;
          return true;
        }
        auto const &name = src_root["user"].asString();
        auto const &email = src_root["email"].asString();
        auto const &pwd = src_root["passwd"].asString();
        std::cout << "name " << name << " email " << email << " pwd " << pwd
                  << std::endl;
        //  查询sql，判断用户是否已存在
        int uid = MySqlManager::get_instance()->register_user(name, email, pwd);
        if (uid <= 0) {
          if (uid == -2) {
            std::cout << " email exist" << std::endl;
          } else if (uid == -3) {

            std::cout << " user exist" << std::endl;
          } else {
            std::cout << "unkonwn error" << std::endl;
          }
          root["error"] = static_cast<int>(ErrorCode::USER_EXISTED);
          std::string jsonstr = root.toStyledString();
          beast::ostream(_connection->response_.body()) << jsonstr;
          return true;
        }
        // 所有操作正确无误，回显给客户端
        root["error"] = 0;
        root["email"] = email;
        root["uid"] = uid;
        root["user"] = name;
        root["passwd"] = pwd;
        root["confirm"] = src_root["confirm"].asString();
        root["vertifycode"] = src_root["vertifycode"].asString();
        std::string jsonstr = root.toStyledString();
        beast::ostream(_connection->response_.body()) << jsonstr;
        return true;
      });
}

LogicSystem::~LogicSystem() {}

bool LogicSystem::handle_get_request(
    std::string _path, std::shared_ptr<HttpConnection> _connection) {
  auto it = get_hanlders_.find(_path);
  if (it == get_hanlders_.end()) {
    return false;
  }
  it->second(_connection);
  return true;
}

bool LogicSystem::handle_post_request(
    std::string _path, std::shared_ptr<HttpConnection> _connection) {
  std::cout << "handle post req: path is " << _path << std::endl;
  auto it = post_handlers_.find(_path);
  if (it == post_handlers_.end()) {
    return false;
  }
  it->second(_connection);
  return true;
}

void LogicSystem::register_get_handler(std::string _path,
                                       http_handler_t handler) {
  if (get_hanlders_.find(_path) == get_hanlders_.end()) {
    get_hanlders_.insert(std::make_pair(_path, handler));
  }
}
void LogicSystem::register_post_handler(std::string _path,
                                        http_handler_t handler) {
  if (post_handlers_.find(_path) == post_handlers_.end()) {
    post_handlers_.insert(std::make_pair(_path, handler));
  }
}
