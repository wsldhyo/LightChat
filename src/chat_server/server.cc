#include "server.hpp"
#include "manager/config_manager.hpp"
#include "manager/redis_manager.hpp"
#include "pool/iocontext_pool.hpp"
#include "session.hpp"
#include "user_manager.hpp"
#include <chrono>
#include <iostream>
Server::Server(asio::io_context &ioc, int port)
    : acceptor_(ioc, tcp::endpoint(tcp::v4(), port)), heartbeat_timer_(ioc) {
  std::cout << "Chat Server start, listening on port: " << port << "\n";
}

Server::~Server() {
  heartbeat_timer_.cancel(); // 析构前需要取消定时器
  close();
}

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

void Server::start_timer() {
  // 设置超时
  heartbeat_timer_.expires_after(std::chrono::seconds(60));
  heartbeat_timer_.async_wait(
      [self = shared_from_this()](boost::system::error_code const &ec) {
        self->on_timer(ec);
      });
}

void Server::stop_timer() {
  std::cout << "stop timer\n";
  heartbeat_timer_.cancel();
}

void Server::on_timer(boost::system::error_code const &ec) {
  if (ec) {
    std::cout << "time error: " << ec.what() << '\n';
    return;
  }
  std::vector<std::shared_ptr<Session>> expired_sessions;
  int32_t session_count{0};

  decltype(sessions_) sessions_copy;  //拷贝出一份，减小锁粒度。避免在加锁期间遍历
  {
    //这里先加线程锁后加分布式锁，与其他流程先加分布式锁后加线程锁的顺序币一样，
    // 所以缓存过期Session随后立即释放线程锁，避免与其他线程加锁顺序不一致引发死锁
    std::lock_guard<std::mutex> lock(mutex_);
    // TODO 是否可考虑分桶检查，而不是一次性检查所有session，降低单次拷贝检查开销。
    // 或者用epoll事件超时机制: epoll_wait 超时返回时（即本轮无任何事件），做一次空闲检测?
    sessions_copy = sessions_;
  }

  auto now = std::chrono::steady_clock::now();
  for (auto it = sessions_copy.begin(); it != sessions_copy.end(); ++it) {
    if (it->second->is_heartbeat_expired(now)) {
      it->second->close();
      expired_sessions.push_back(it->second);
    } else {
      ++session_count;
    }
  }

  //设置session数量
  auto cfg = ConfigManager::get_instance();
  auto self_name = (*cfg)["SelfServer"]["Name"];
  auto count_str = std::to_string(session_count);
  RedisMgr::get_instance()->h_set(REDIS_LOGIN_COUNT_PREFIX, self_name,
                                  count_str);

  for (auto &session : expired_sessions) {
    session->deal_exception_session();
  }
  // 再次启动定时器
  start_timer();
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