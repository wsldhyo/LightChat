#include "grpc_vertify_code_client.hpp"
#include "constant.hpp"
#include "message.grpc.pb.h"
#include "rpc_connection_pool.hpp"
#include <grpcpp/channel.h>
#include <grpcpp/create_channel.h>
#include <grpcpp/security/credentials.h>
#include <grpcpp/support/status.h>
#include <memory>
#include "config_manager.hpp"
#include "rpc_connection_pool.hpp"
GrpcVertifyCodeClient::GrpcVertifyCodeClient() {
  auto& config_mgr = *ConfigManager::get_instance();
  std::string host = config_mgr["VertifyServer"]["host"];
  std::string port = config_mgr["VertifyServer"]["port"];
  pool_.reset(new RpcConnectionPool(std::thread::hardware_concurrency(), host, port));
}

GetVertifyRsp
GrpcVertifyCodeClient::get_vertify_code(std::string const &_email) {
  ClientContext client_context;
  GetVertifyReq request;
  GetVertifyRsp reponse;
  request.set_email(_email);
  // RPC 远端调用 服务端的GetVertifyCode
  auto stub = pool_->get_connection();
  auto status = stub->GetVertifyCode(&client_context, request, &reponse);

  if (status.ok()) {
    pool_->return_connection(std::move(stub));
    return reponse;
  } else {
    reponse.set_error(static_cast<int>(ErrorCode::RPC_FAILED));
    return reponse;
  }
}