#include "chat_rpc_server.hpp"
#include "logic_system.hpp"
#include "manager/config_manager.hpp"
#include "manager/redis_manager.hpp"
#include "pool/iocontext_pool.hpp"
#include "server.hpp"
#include "utility/constant.hpp"
#include "utility/defer.hpp"
#include "utility/toolfunc.hpp"
#include <boost/asio/signal_set.hpp>
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <string>

int read_config(int argc, char *argv[]) {

  // 默认配置文件
  std::string chat_config = "chat_server_config.ini";

  // 根据进程参数选择不同配置
  if (argc > 1) {
    std::string arg = argv[1];
    if (arg == "2") {
      chat_config = "chat_server2_config.ini";
    } else {
      std::cerr << "Unknown argument: " << arg << std::endl;
      return 1;
    }
  }
  auto cfg = ConfigManager::get_instance();
  cfg->parse(chat_config);
  cfg->parse("basic_config.ini"sv);
  return 0;
}


int main(int argc, char *argv[]) {
  read_config(argc, argv);
  int port{0};
  std::string server_address;
  std::string server_name;
  {
    auto cfg = ConfigManager::get_instance();
    auto res = string_to_int((*cfg)["SelfServer"]["port"], port);
    if (res != ErrorCodes::NO_ERROR || port <= 0 || port > 65535) {
      std::cout << "Get port error: port is " << port << '\n';
    }

    server_address =
        (*cfg)["SelfServer"]["host"] + ":" + (*cfg)["SelfServer"]["rpcport"];
    server_name = (*cfg)["SelfServer"]["name"];
  }

  try {
    // 将登录数设置为0
    RedisMgr::get_instance()->h_set(REDIS_LOGIN_COUNT_PREFIX, server_name, "0");
    Defer derfer([server_name]() {
      RedisMgr::get_instance()->h_del(REDIS_LOGIN_COUNT_PREFIX, server_name);
      RedisMgr::get_instance()->close();
    });

    boost::asio::io_context io_context;
    auto pool = IoContextPool::get_instance();

    ChatServiceImpl rpc_service; // Grpc服务
    grpc::ServerBuilder builder;

    // 监听端口和添加服务
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&rpc_service);

    // 构建 gRPC 服务器
    std::unique_ptr<grpc::Server> rpc_server(builder.BuildAndStart());
    auto rpc_ptr = rpc_server.get();  //  保存裸指针供异步回调使用

    std::cout << "RPC Server listening on " << server_address << std::endl;

    // 单独启动一个线程处理grpc服务
    std::thread grpc_server_thread([rpc_ptr]() { rpc_ptr->Wait(); });

    auto server = std::make_shared<Server>(io_context, port);

    //  shared_ptr 持有 signals 避免悬空
    auto signals = std::make_shared<boost::asio::signal_set>(io_context, SIGINT, SIGTERM);
    signals->async_wait([signals, server, &io_context, pool, rpc_ptr](auto, auto) {
      std::cout << "Received termination signal, shutting down..." << std::endl;

      server->stop_timer();       // 停止所有定时器（内部调用 cancel）
      pool->stop();               // 停止线程池
      rpc_ptr->Shutdown();        // 通知 gRPC 服务退出

      std::this_thread::sleep_for(std::chrono::milliseconds(50)); //  等待 stop_timer中的cancel 回调
      io_context.stop();          // 最后停止事件循环
    });

    LogicSystem::get_instance()->set_server(server);
    rpc_service.RegisterServer(server);

    server->start_accept();  // 启动监听
    server->start_timer();   

    io_context.run();        // 阻塞直到 stop() 被调用
    grpc_server_thread.join(); // 等待 gRPC 服务线程退出

    return 0;
  } catch (std::exception &e) {
    std::cerr << "main func: exception occurred: " << e.what() << std::endl;
  }

  return 0;
}
