#include "vertify_rpc_client.hpp"
#include "manager/config_manager.hpp"
#include "utility/constant.hpp"

using grpc::ClientContext;
using grpc::Status;
using message::GetVertifyReq;
VertifyRPCClient::VertifyRPCClient() {
  // 读取配置的RPC服务器地址
  auto config_mgr = ConfigManager::get_instance();
  std::string const &host = (*config_mgr)["VertifyServer"]["host"];
  std::string const &port = (*config_mgr)["VertifyServer"]["port"];
  //初始化连接池
  pool_.reset(new RPCConnPool(5, host, port));
}

GetVertifyRsp VertifyRPCClient::get_vertify_code(std::string const &email) {
  ClientContext context;
  GetVertifyRsp response;
  GetVertifyReq request;
  request.set_email(email);
  // 从连接池获取 Stub
  auto stub = pool_->get_connection();

  // RPC远端调用
  Status status = stub->GetVertifyCode(&context, request, &response);
  if (!status.ok()) {
    response.set_error(static_cast<int32_t>(ErrorCodes::RPC_CALL_FAILED));
  }
  // 归还 Stub 到连接池
  pool_->return_connection(std::move(stub));
  return response;
}