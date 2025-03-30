#ifndef GRPC_VERTIFY_CODE_CLIENT_HPP
#define GRPC_VERTIFY_CODE_CLIENT_HPP
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/support/status.h>
#include <memory>
#include <string>

#include "message.pb.h"
#include "rpc_connection_pool.hpp"
#include "singleton.hpp"
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using message::GetVertifyReq;
using message::GetVertifyRsp;
using message::VertifyService;
class GrpcVertifyCodeClient : public Singleton<GrpcVertifyCodeClient>{
public:
   friend class Singleton<GrpcVertifyCodeClient>; 
  GetVertifyRsp get_vertify_code(std::string const& email);

private:
    GrpcVertifyCodeClient();
  std::unique_ptr<RpcConnectionPool> pool_;

};

#endif