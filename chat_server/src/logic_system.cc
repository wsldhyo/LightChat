
#include <iostream>

#include <json/reader.h>
#include <json/value.h>
#include <json/writer.h>

#include "logic_system.hpp"
#include "session.hpp"

LogicSystem::LogicSystem() : b_stop_(false) {
  regiser_callback();
  work_thread_ = std::thread(&LogicSystem::deal_message, this);
}

LogicSystem::~LogicSystem() {
  b_stop_ = true;
  cond_.notify_one();
  work_thread_.join();
}

void LogicSystem::post_msg_to_que(std::shared_ptr<LogicNode> _logic_node) {
  if (b_stop_) {
    return;
  }
  std::lock_guard<std::mutex> gaurd(msg_que_lock_);
  if (msg_queue_.size() > TCP_MAX_LOGIG_QUE_SIZE) {
    std::cout << "Post msg to queue failed. queue is full. Session: "
              << _logic_node->session_->get_uuid() << std::endl;
    return;
  }

  msg_queue_.push(_logic_node);
  if (msg_queue_.size() == 1) {
    cond_.notify_one();
  }
}

void LogicSystem::regiser_callback() {
  // 注册登录聊天服务器请求
  callbacks_[RequestID::LOGIN_CHAT_SERVER] = [](std::shared_ptr<Session> _session,
                                    RequestID _id, std::string const &_msg) {
    Json::Reader reader;
    Json::Value root;
    reader.parse(_msg, root);
    std::cout << "user login uid is  " << root["uid"].asInt()
              << " user token  is " << root["token"].asString() << std::endl;
    std::string return_str = root.toStyledString();
    _session->send(return_str, static_cast<int>(_id));
  };
}

void LogicSystem::deal_message() {
  while (true) {
    {
      std::unique_lock<std::mutex> gaurd(msg_que_lock_);
      while (msg_queue_.empty() && !b_stop_) {
        cond_.wait_for(gaurd, std::chrono::milliseconds(500));
      }
    }

    if (b_stop_) {
      // 判断是否为关闭状态，把所有逻辑执行完后则退出循环
      while (!msg_queue_.empty()) {
        auto msg_node = msg_queue_.front();
        std::cout << "recv_msg id  is " << msg_node->recvnode_->msg_id_
                  << std::endl;
        auto call_back_iter = callbacks_.find(
            static_cast<RequestID>(msg_node->recvnode_->msg_id_));
        if (call_back_iter == callbacks_.end()) {
          msg_queue_.pop();
          continue;
        }
        call_back_iter->second(
            msg_node->session_,
            static_cast<RequestID>(msg_node->recvnode_->msg_id_),
            std::string(msg_node->recvnode_->data_,
                        msg_node->recvnode_->cur_len_));
        msg_queue_.pop();
      }
      break;
    }

    // 如果没有停服，且说明队列中有数据, 取出数据并发送给对端
    auto msg_node = msg_queue_.front();
    std::cout << "recv_msg id  is " << msg_node->recvnode_->msg_id_
              << std::endl;
    auto call_back_iter =
        callbacks_.find(static_cast<RequestID>(msg_node->recvnode_->msg_id_));
    if (call_back_iter == callbacks_.end()) {
      msg_queue_.pop();
      std::cout << "msg id [" << msg_node->recvnode_->msg_id_
                << "] handler not found" << std::endl;
      continue;
    }
    call_back_iter->second(
        msg_node->session_,
        static_cast<RequestID>(msg_node->recvnode_->msg_id_),
        std::string(msg_node->recvnode_->data_, msg_node->recvnode_->cur_len_));
    msg_queue_.pop();
  }
}