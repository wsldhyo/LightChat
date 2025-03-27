#include "grpc_status_client.hpp"
#include "config_manager.hpp"
#include "grpc_status_connection_pool.hpp"
#include "../common//defer.hpp"
#include "../common/constant.hpp"
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/status.h>
using grpc::ClientContext;
using grpc::Status;

using message::GetChatServerReq;
using message::LoginReq;
GrpcStatusClient::GrpcStatusClient(){
	auto sp_confid_mgr = ConfigManager::get_instance();
    auto& config_mgr = *sp_confid_mgr;
	std::string host = config_mgr["StatusServer"]["host"];
	std::string port = config_mgr["StatusServer"]["port"];
	pool_.reset(new StatusConPool(std::thread::hardware_concurrency(), host, port));
}

GrpcStatusClient::~GrpcStatusClient(){}
	
GetChatServerRsp GrpcStatusClient::GetChatServer(int uid){
	ClientContext context;
	GetChatServerRsp reply;
	GetChatServerReq request;
	request.set_uid(uid);
	auto stub = pool_->get_connection();
    // RPC调用，查询服务uid的聊天服务器
	Status status = stub->GetChatServer(&context, request, &reply);
	Defer defer([&stub, this]() {
		pool_->return_connection(std::move(stub));
		});
	if (status.ok()) {	
		return reply;
	}
	else {
		reply.set_error(static_cast<int>(ErrorCode::RPC_FAILED));
		return reply;
	}
}

	LoginRsp GrpcStatusClient::Login(int uid, std::string token){
	ClientContext context;
	LoginRsp reply;
	LoginReq request;
	request.set_uid(uid);
	request.set_token(token);

	auto stub = pool_->get_connection();
	Status status = stub->Login(&context, request, &reply);
	Defer defer([&stub, this]() {
		pool_->return_connection(std::move(stub));
		});
	if (status.ok()) {
		return reply;
	}
	else {
		reply.set_error(static_cast<int>(ErrorCode::RPC_FAILED));
		return reply;
	}
    }