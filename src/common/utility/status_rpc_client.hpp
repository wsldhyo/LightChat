#ifndef STATUS_RPC_CLIENT_HPP
#define STATUS_RPC_CLIENT_HPP
#include "pool/status_server_conn_pool.hpp"
#include "singleton.hpp"
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