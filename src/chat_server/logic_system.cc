#include "logic_system.hpp"
#include "manager/mysql_manager.hpp"
#include "session.hpp"
#include "utility/constant.hpp"
#include "utility/defer.hpp"
#include "utility/status_rpc_client.hpp"
#include <iostream>
#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>

LogicSystem::LogicSystem() : b_stop_(false) {
  register_msg_handler();
  work_thread_ = std::thread(&LogicSystem::deal_msg, this);
}

LogicSystem::~LogicSystem() {
  b_stop_ = true;
  deal_msg_cond_.notify_one();
  if (work_thread_.joinable()) {
    work_thread_.join();
  }
}

void LogicSystem::post_msg(std::unique_ptr<RecvMsgNode> msg,
                           std::shared_ptr<Session> session) {
  {
    std::lock_guard<std::mutex> lock(msg_que_mutex_);
    auto que_size{recv_msg_que_.size()};
    if (que_size > TCP_MAX_LOGIG_QUE_SIZE) {
      std::cout << "Logic queue overflow. msg;[" << msg->data_
                << "] will be discarded\n";
      return;
    }
    recv_msg_que_.emplace(std::move(msg), session);
    // 队列为空时插入，则通知后台线程开始处理
    if (que_size == 0) {
      deal_msg_cond_.notify_one();
    }
  }
}

void LogicSystem::deal_msg() {
  decltype(recv_msg_que_) batch_msg;
  LogicNode logic_node;
  while (!b_stop_) {
    {
      {
        std::unique_lock<std::mutex> lock(msg_que_mutex_);
        while (recv_msg_que_.empty() && !b_stop_) {
          deal_msg_cond_.wait_for(lock, std::chrono::milliseconds{500});
        }
        batch_msg.swap(recv_msg_que_);
      }

      while (!batch_msg.empty()) {
        logic_node = std::move(batch_msg.front());
        batch_msg.pop();
        std::cout << "recv_msg id  is " << logic_node.msg_->msg_id_
                  << std::endl;
        auto call_back_iter =
            handlers_.find(static_cast<ReqId>(logic_node.msg_->msg_id_));
        if (call_back_iter == handlers_.end()) {
          std::cout << "msg id [" << logic_node.msg_->msg_id_
                    << "] handler not found" << std::endl;
          continue;
        }

        call_back_iter->second(
            logic_node.session_, static_cast<ReqId>(logic_node.msg_->msg_id_),
            std::string_view(logic_node.msg_->data_, logic_node.msg_->length_));
      }
    }
  }
}

void LogicSystem::register_msg_handler() {
  // TODO 这里没必要再请求一次StatusServer进行验证，
  //利用JWT验证流程，用同一个Secret验证id和token

  // 注册登录聊天服务器请求
  handlers_.emplace(
      ReqId::ID_CHAT_LOGIN, std::move([this](std::shared_ptr<Session> session,
                                             ReqId id, std::string_view msg) {
        Json::Reader reader;
        Json::Value root;
        reader.parse(msg.data(), msg.data() + msg.size(), root);
        auto uid = root["uid"].asInt();
        std::cout << "user login uid is  " << uid << " user token  is "
                  << root["token"].asString() << std::endl;

        //从状态服务器获取token, 验证token是否准确
        auto rsp = StatusGrpcClient::getinstance()->login(
            uid, root["token"].asString());
        Json::Value rtvalue;
        Defer defer([this, &rtvalue, session]() {
          std::string return_str = rtvalue.toStyledString();
          session->send(return_str,
                        static_cast<uint16_t>(ReqId::ID_CHAT_LOGIN));
        });

        rtvalue["error"] = rsp.error();
        if (rsp.error() != static_cast<int32_t>(ErrorCodes::NO_ERROR)) {
          return;
        }

        UserInfo *user_info{};
        //内存中查询用户信息
        auto find_iter = users_.find(uid);
        if (find_iter == users_.end()) {
          //查询数据库
          auto user_info_opt = MysqlMgr::getinstance()->get_user(uid);
          if (user_info_opt == std::nullopt) {
            rtvalue["error"] = static_cast<int32_t>(ErrorCodes::UID_INVALID);
            return;
          }

          users_[uid] = user_info_opt.value();
          user_info = &users_[uid];

        } else {
          user_info = &find_iter->second;
        }

        rtvalue["uid"] = uid;
        rtvalue["token"] = rsp.token();
        rtvalue["name"] = user_info->name;
      }));
}