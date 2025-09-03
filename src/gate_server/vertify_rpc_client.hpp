#ifndef VERTIFY_RPC_CLIENT
#define VERTIFY_RPC_CLIENT
#include "pool/rpc_conn_pool.hpp"
#include "utility/singleton.hpp"

/**
 * @brief gRPC 验证码客户端，单例模式
 *
 * 功能：
 * - 内部维护 RPCConnPool 连接池
 * - 提供 GetVertifyCode 接口调用 RPC 服务
 */
class VertifyRPCClient : public Singleton<VertifyRPCClient> {
  friend class Singleton<VertifyRPCClient>;

public:
  /**
   * @brief 获取指定邮箱的验证码
   * @param email 待发送验证码的邮箱
   * @return GetVertifyRsp RPC 响应对象，包含错误码
   */
  GetVertifyRsp GetVertifyCode(std::string const &email);

private:
  VertifyRPCClient();

private:
  std::unique_ptr<RPCConnPool> pool_; ///< 内部 RPC 连接池
};

#endif