#include "../../common/config_manager.hpp"
#include "../../common/redis_connection_manager.hpp"
#include "status_server_impl.hpp"
#include <boost/asio/io_context.hpp>
#include <boost/asio/signal_set.hpp>
#include <grpcpp/server_builder.h>
#include <thread>

void run_server() {
  auto sp_config = ConfigManager::get_instance();
  auto &cfg = *sp_config;
  cfg.parse();
  std::string server_address(cfg["StatusServer"]["host"] + ":" +
                             cfg["StatusServer"]["port"]);
  std::cout << "server addr: " << server_address << std::endl;
  StatusServiceImpl service;

  grpc::ServerBuilder builder;
  // 监听端口和添加服务
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  // 构建并启动gRPC服务器
  std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // 创建Boost.Asio的io_context
  boost::asio::io_context io_context;
  // 创建signal_set用于捕获SIGINT
  boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);

  // 设置异步等待SIGINT信号
  signals.async_wait(
      [&server, &io_context](const boost::system::error_code &error,
                             int signal_number) {
        if (!error) {
          std::cout << "Shutting down server..." << std::endl;
          server->Shutdown(); // 优雅地关闭服务器
          io_context.stop();  // 停止io_context
        }
      });

  // 在单独的线程中运行io_context
  std::thread([&io_context]() {
    std::cout << "ioc run" << std::endl;
    io_context.run();
  }).detach();

  // 等待服务器关闭
  server->Wait();
}

int main(int argc, char **argv) {
  try {
    run_server();
    RedisConnectionManager::get_instance()->close();
  } catch (std::exception const &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    RedisConnectionManager::get_instance()->close();
    return EXIT_FAILURE;
  }

  return 0;
}