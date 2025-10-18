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
    //将登录数设置为0
    RedisMgr::get_instance()->h_set(REDIS_LOGIN_COUNT_PREFIX, server_name, "0");
    Defer derfer([server_name]() {
      RedisMgr::get_instance()->h_del(REDIS_LOGIN_COUNT_PREFIX, server_name);
      RedisMgr::get_instance()->close();
    });

    boost::asio::io_context io_context;
    auto pool = IoContextPool::get_instance();
    //定义一个GrpcServer
    ChatServiceImpl service;
    grpc::ServerBuilder builder;
    // 监听端口和添加服务
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    // 构建并启动gRPC服务器
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "RPC Server listening on " << server_address << std::endl;

    //单独启动一个线程处理grpc服务
    std::thread grpc_server_thread([&server]() { server->Wait(); });

    boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
    signals.async_wait([&io_context, pool, &server](auto, auto) {
      io_context.stop();
      pool->stop();
      server->Shutdown();
    });

    auto s = std::make_shared<Server>(io_context, port);
    LogicSystem::get_instance()->set_server(s);
    service.RegisterServer(s);
    s->start_accept();
    io_context.run();
    grpc_server_thread.join();
    return 0;
  } catch (std::exception &e) {
    std::cerr << "main func: exception occurred: " << e.what() << std::endl;
  }

  return 0;
}