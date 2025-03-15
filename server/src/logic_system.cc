#include "logic_system.hpp"
#include "../../common/constant.hpp"
#include "http_connection.hpp"
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

  register_post_handler(
      "/get_vertifycode", [](std::shared_ptr<HttpConnection> _connection) {
        auto body_str =
            beast::buffers_to_string(_connection->request_.body().data());
        std::cout << "receive get_vertifycode body is " << body_str << std::endl;
        _connection->response_.set(http::field::content_type, "text/json");
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

        auto email = src_root["email"].asString();
        std::cout << "email is " << email << std::endl;
        res_root["error"] = 0;
        res_root["email"] = email;
        beast::ostream(_connection->response_.body()) << res_root.toStyledString();
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
