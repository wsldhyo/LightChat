#ifndef IOCONTEXT_POOL_HPP
#define IOCONTEXT_POOL_HPP
#include "utility/singleton.hpp"
#include <boost/asio/executor_work_guard.hpp>
#include <boost/asio/io_context.hpp>
#include <thread>

/**
 * @brief IoContextPool 管理多个 boost::asio::io_context 并分配线程执行。
 *
 * 提供多线程环境下的 IO 服务池，保证异步操作可以并行执行。
 * 单例模式管理，线程安全分发 io_context。
 */
class IoContextPool : public Singleton<IoContextPool> {
  friend Singleton<IoContextPool>;

public:
  using IOContext_t = boost::asio::io_context;
  using work_t = boost::asio::executor_work_guard<IOContext_t::executor_type>;
  using work_uptr_t = std::unique_ptr<work_t>;

  ~IoContextPool();

  /**
   * @brief 获取一个 io_context，循环分配。
   * @return boost::asio::io_context&
   * @note 线程不安全，不要多线程调用
   */
  IOContext_t &get_iocontext();

  /**
   * @brief 停止所有 io_context，并等待线程结束。
   * @note 线程不安全，不要多线程调用
   */
  void stop();

private:
  IoContextPool(std::size_t size = std::thread::hardware_concurrency());
  std::vector<IOContext_t> io_contexts_;

  /*
   保证 io_context 的 run() 不会因为没有任务而立即退出
   当 work_guard 调用 reset，将解除保持 io_context 运行的作用:
        如果 io_context 没有其他任务，run() 可以正常返回
   另外work_gaurd对象销毁时，也会自动调用reset解除ioc的run保持
  */
  std::vector<work_uptr_t> works_;

  std::vector<std::thread> threads_; //执行 io_context 的线程
  std::size_t next_iocontext_; // 索引，每次从iocontexts_中取出该位置的ioc
};
#endif