#include "status_rpc_client.hpp"
#include "manager/config_manager.hpp"
#include "utility/defer.hpp"
#include <iostream>

using grpc::ClientContext;
using grpc::Status;
using message::GetChatServerReq;
using message::LoginReq;
using message::LoginRsp;

StatusGrpcClient::StatusGrpcClient() {
  auto cfg = ConfigManager::getinstance();
  std::string host = (*cfg)["StatusServer"]["host"];
  std::string port = (*cfg)["StatusServer"]["port"];
  pool_ = std::make_unique<StatusConnPool>(5, host, port);
}

GetChatServerRsp StatusGrpcClient::get_chatserver(int uid) {
  ClientContext context;
  GetChatServerRsp reply;
  GetChatServerReq request;
  request.set_uid(uid);
  auto stub = pool_->get_connection();
  Status status = stub->GetChatServer(&context, request, &reply);
  std::cout << "get chat server success\n";
  Defer defer([&stub, this]() { pool_->return_connection(std::move(stub)); });
  if (status.ok()) {
    return reply;
  } else {
    reply.set_error(static_cast<int32_t>(ErrorCodes::RPC_CALL_FAILED));
    return reply;
  }
}

LoginRsp StatusGrpcClient::login(int uid, std::string token) {
  ClientContext context;
  LoginRsp reply;
  LoginReq request;
  request.set_uid(uid);
  request.set_token(token);

  auto stub = pool_->get_connection();
  Status status = stub->Login(&context, request, &reply);
  Defer defer([&stub, this]() { pool_->return_connection(std::move(stub)); });
  if (status.ok()) {
    return reply;
  } else {
    reply.set_error(static_cast<int32_t>(ErrorCodes::RPC_CALL_FAILED));
    return reply;
  }
}
