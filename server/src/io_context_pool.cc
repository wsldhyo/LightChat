#include "io_context_pool.hpp"
IOContextPool::IOContextPool(
    std::size_t _thread_num /* std::thread::hardware_concurrency()*/)
    : io_contexts_(_thread_num),works_(_thread_num), next_io_context_index_(0) {

  for (std::size_t i = 0; i < _thread_num; ++i) {
    works_.push_back(std::make_unique<work_t>(
        boost::asio::make_work_guard(io_contexts_[i])));
  }

  for (std::size_t i = 0; i < _thread_num; ++i) {
    threads_.emplace_back([this, i]() { io_contexts_[i].run(); });
  }
}

IOContextPool::~IOContextPool() { stop(); }

IOContextPool::io_context_t &IOContextPool::get_io_context() {
  auto &io_context = io_contexts_[next_io_context_index_++];
  if (next_io_context_index_ == io_contexts_.size()) {
    next_io_context_index_ = 0;
  }
  return io_context;
}

void IOContextPool::stop() {
  for (auto &work : works_) {
    work->get_executor()
        .context()
        .stop(); // 先停止io_context服务，让其冲io_context.run()的状态中退出，不再派发事件回调
    work.reset(); // 销毁work，同时连带销毁io_context
  }

  for (auto &thread : threads_) {
    thread.join();
  }
}
