#ifndef SERVER_HPP
#define SERVER_HPP
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <unordered_map>
namespace asio = boost::asio;
using asio::ip::tcp;
class Session;

/**
 * @brief TCP 服务器，负责管理所有 Session 的生命周期。
 * - Server 自身由 shared_ptr 管理，保证异步回调中不会悬空
 * - Session 使用 UUID 存放在 sessions_ 中，便于快速定位/移除
 * - 关闭时acceptor 先关闭，防止新连接，再逐一关闭所有 Session
 * - 线程安全，sessions_ 用 mutex 保护，但移除操作通过 post 异步回到
 *   Server 所属的 io_context 中，避免死锁和重入
 */
class Server : public std::enable_shared_from_this<Server> {
public:
  /**
   * @brief 构造函数
   * @param ioc 使用的 io_context
   * @param port 监听端口
   *
   * - 初始化 acceptor 监听端口
   * - 打印启动信息
   */
  Server(asio::io_context &ioc, int port);

  ~Server();

  /**
   * @brief 启动异步 accept 循环
   *
   * - 每 accept 一个新连接，就创建一个 Session 并放入 sessions_
   * - 然后递归调用 start_accept()，保持持续监听
   */
  void start_accept();

  /**
   * @brief 真正移除某个 Session（仅限在 io_context 线程中调用）
   */
  void remove_session(std::string const &session_id);

  bool check_session_vaild(std::string const& session_id);

  void on_timer(boost::system::error_code const& ec);
private:
  /// 将函数投递到 acceptor 的执行上下文
  template <typename F> void post(F &&f) {
    asio::post(acceptor_.get_executor(), std::forward<F>(f));
  }


  /**
   * @brief 关闭服务器
   *
   * - 幂等操作，compare_exchange_strong 保证只执行一次
   * - 先关闭 acceptor，阻止新连接
   * - 再把 sessions_ move 出来，在无锁状态下逐个调用 session->close()
   *   避免持锁期间触发回调造成死锁
   */
  void close();

  tcp::acceptor acceptor_; ///< TCP 监听器
  std::unordered_map<std::string, std::shared_ptr<Session>>
      sessions_;                    ///< 活跃会话
  std::mutex mutex_;                ///< 保护 sessions_ 的互斥锁
  std::atomic_bool closing_{false}; ///< 是否正在关闭，防止重复执行

  asio::steady_timer timer_;  // 定时器
};
#endif