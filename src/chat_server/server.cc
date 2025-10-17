#include "server.hpp"
#include "pool/iocontext_pool.hpp"
#include "session.hpp"
#include "user_manager.hpp"
#include <iostream>
Server::Server(asio::io_context &ioc, int port)
    : acceptor_(ioc, tcp::endpoint(tcp::v4(), port)) {
  std::cout << "Chat Server start, listening on port: " << port << "\n";
}

Server::~Server() { close(); }

void Server::remove_session(std::string const &session_id) {
  // 同步移除UserManager中的绑定信息
  if (sessions_.find(session_id) != sessions_.end()) {
    UserManager::get_instance()->remove_user_session(
        sessions_[session_id]->get_user_id());
  }
  std::lock_guard<std::mutex> lock(mutex_);
  sessions_.erase(session_id);
}

void Server::start_accept() {

  auto self = shared_from_this();
  auto &ioc = IoContextPool::get_instance()->get_iocontext();
  auto peer = std::make_shared<tcp::socket>(
      ioc); // 这里要用shared_ptr，保证回调调用前socket有效，不能move到lambda内
  acceptor_.async_accept(*peer, [self, peer](boost::system::error_code ec) {
    if (!ec) {
      std::cout << "new connection reach, peer:" << peer->remote_endpoint()
                << '\n';
      auto new_session = std::make_shared<Session>(std::move(*peer), self);
      {
        std::lock_guard<std::mutex> lock(self->mutex_);
        self->sessions_.insert(
            std::make_pair(new_session->get_session_id(), new_session));
      }
      new_session->start();
    } else {
      std::cout << "server accept error: " << ec.what() << '\n';
    }
    self->start_accept();
  });
}

bool Server::check_session_vaild(std::string const &session_id) {
  std::lock_guard<std::mutex> lock(mutex_);
  return sessions_.find(session_id) != sessions_.end();
}

void Server::close() {
  bool expected = false;
  if (!closing_.compare_exchange_strong(expected, true)) {
    return; // 已经在关闭中了，幂等
  }

  // 先取消/关闭 acceptor，阻止新 accept
  boost::system::error_code ec;
  acceptor_.cancel(ec);
  acceptor_.close(ec);

  // 将 sessions_ 搬到本地，避免在持锁时调用 session->close() 导致死锁/重入问题
  std::unordered_map<std::string, std::shared_ptr<Session>> local;
  {
    std::scoped_lock lk(mutex_);
    local = std::move(sessions_); // move 出来
    sessions_.clear();
  }

  // 在不持锁的情况下关闭每个 session
  for (auto &kv : local) {
    auto &sess = kv.second;
    if (sess) {
      sess->close(); // 由 Session 提供的关闭方法（幂等）
    }
  }
  // local 在离开作用域会析构 shared_ptr（如果没有其他持有者则释放）
}