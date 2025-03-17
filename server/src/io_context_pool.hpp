#ifndef IO_CONTEXT_POOL_HPP
#define IO_CONTEXT_POOL_HPP
#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>
#include "../common/singleton.hpp"
#include <thread>
#include <memory>
class IOContextPool : public Singleton<IOContextPool>{
public:
     friend class Singleton<IOContextPool>;
     using io_context_t = boost::asio::io_context;
     using work_t = boost::asio::executor_work_guard<io_context_t::executor_type>;
     using work_uptr_t = std::unique_ptr<work_t>;

     ~IOContextPool();
     io_context_t& get_io_context();
     void stop();

private:
    IOContextPool(std::size_t thread_num = std::thread::hardware_concurrency());

    std::vector<io_context_t> io_contexts_;
    std::vector<work_uptr_t> works_;
    std::vector<std::thread> threads_;
    std::size_t next_io_context_index_;
}; 
#endif