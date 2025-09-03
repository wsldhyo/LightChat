#include "iocontext_pool.hpp"
#include <iostream>

IoContextPool::IoContextPool(std::size_t size)
    : io_contexts_(size), works_(0), next_iocontext_(0) {

  works_.reserve(size);

  for (std::size_t i = 0; i < size; ++i) {
    works_.emplace_back(std::make_unique<work_t>(
        boost::asio::make_work_guard(io_contexts_[i])));
  }

  //创建多个线程，每个线程内部启动iocontext
  for (std::size_t i = 0; i < io_contexts_.size(); ++i) {
    threads_.emplace_back([this, i]() { io_contexts_[i].run(); });
  }
}

IoContextPool::~IoContextPool() {
  stop();
  std::cout << "IoContextPool destruct" << '\n';
}

boost::asio::io_context &IoContextPool::get_iocontext() {
  auto &service = io_contexts_[next_iocontext_++];
  if (next_iocontext_ == io_contexts_.size()) {
    next_iocontext_ = 0;
  }
  return service;
}

void IoContextPool::stop() {
  //因为仅仅执行work.reset并不能让iocontext从run的状态中退出
  //当iocontext已经绑定了读或写的监听事件后，还需要手动stop该服务。
  for (auto &work : works_) {
    work->get_executor()
        .context()
        .stop();   // 停止ioc的事件循环，所有run会尽快返回 
    work->reset(); // 销毁work_gaurd，不再阻止ioc.run无任务的退出 
  }

  for (auto &t : threads_) {
    t.join();
  }
}