#ifndef GRPC_STATUS_CLIENT_HPP
#define GRPC_STATUS_CLIENT_HPP
#include "../common/singleton.hpp"
#include "message.grpc.pb.h"
using message::GetChatServerRsp;
using message::LoginRsp;
class StatusConPool;
class GrpcStatusClient :public Singleton<GrpcStatusClient>
{
	friend class Singleton<GrpcStatusClient>;
public:
	~GrpcStatusClient() ;


	GetChatServerRsp GetChatServer(int uid);
	LoginRsp Login(int uid, std::string token);
private:
	GrpcStatusClient();
	std::unique_ptr<StatusConPool> pool_;
	
};
#endif