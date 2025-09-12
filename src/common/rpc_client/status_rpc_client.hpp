#ifndef STATUS_RPC_CLIENT_HPP
#define STATUS_RPC_CLIENT_HPP
#include "message.grpc.pb.h"
#include "message.pb.h"
#include "utility/singleton.hpp"
#include "pool/status_server_conn_pool.hpp"

using message::GetChatServerRsp;
using message::LoginRsp;
class StatusGrpcClient : public Singleton<StatusGrpcClient> {
  friend class Singleton<StatusGrpcClient>;

public:
  ~StatusGrpcClient() {}
  GetChatServerRsp get_chatserver(int uid);
  LoginRsp login(int uid, std::string token);

private:
  StatusGrpcClient();
  std::unique_ptr<StatusConnPool> pool_;
};

#endif