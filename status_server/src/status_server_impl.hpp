#ifndef STATUS_SERVER_HPP
#define STATUS_SERVER_HPP
#include "message.grpc.pb.h"
#include <grpcpp/channel.h>
#include <grpcpp/server_context.h>
#include <grpcpp/support/status.h>
#include <string>

using grpc::Channel;
using grpc::ServerContext;
using grpc::Status;
using message::StatusService;
using message::GetChatServerRsp;
using message::GetChatServerReq;
using message::LoginRsp;
using message::LoginReq;
class  ChatServer {
public:
	ChatServer():host(""),port(""),name(""),con_count(0){}
	ChatServer(const ChatServer& cs):host(cs.host), port(cs.port), name(cs.name), con_count(cs.con_count){}
	ChatServer& operator=(const ChatServer& cs) {
		if (&cs == this) {
			return *this;
		}

		host = cs.host;
		name = cs.name;
		port = cs.port;
		con_count = cs.con_count;
		return *this;
	}
	std::string host;
	std::string port;
	std::string name;
	int con_count;
};
class StatusServiceImpl final : public StatusService::Service
{
public:
	StatusServiceImpl();
	Status GetChatServer(ServerContext* context, const GetChatServerReq* request,
		GetChatServerRsp* reply) override;
	Status Login(ServerContext* context, const LoginReq* request,
		LoginRsp* reply) override;
private:
	void insert_token(int uid, std::string token);
	ChatServer get_chat_server();
	std::unordered_map<std::string, ChatServer> _servers;
	std::mutex _server_mtx;

};


#endif