#ifndef RPC_CONN_POOL_HPP
#define RPC_CONN_POOL_HPP
#include <grpcpp/grpcpp.h>
#include "message.grpc.pb.h"
#include <queue>
#include <condition_variable>

using grpc::Channel;
using grpc::Status;
using grpc::ClientContext;

// messgae.proto定义的类
using message::GetVertifyReq;  
using message::GetVertifyRsp;
using message::VertifyService;



/**
 * @brief RPC 连接池，用于管理 gRPC 客户端连接，避免频繁创建和销毁 Stub。
 * 
 * 功能：
 * - 初始化指定数量的 gRPC Stub 并存入队列
 * - 提供获取和归还连接的接口
 * - 支持关闭连接池，停止获取连接
 */
class RPCConnPool {
public:
    /**
     * @brief 构造函数，创建指定数量的 gRPC Stub 并放入连接池
     * @param pool_size 连接池大小
     * @param host RPC 服务器地址
     * @param port RPC 服务器端口
     */
	RPCConnPool(size_t pool_size, std::string host, std::string port);
		
        
    /**
     * @brief 析构函数，关闭连接池并释放所有 Stub
     */
	~RPCConnPool() ;

    /**
     * @brief 获取一个可用的 gRPC 连接
     * @return std::unique_ptr<VertifyService::Stub> 返回 Stub 对象，若连接池已停止则返回 nullptr
     */
	std::unique_ptr<VertifyService::Stub> get_connection(); 

    /**
     * @brief 将使用完的连接归还到连接池
     * @param context 待归还的 Stub 对象
     */
	void return_connection(std::unique_ptr<VertifyService::Stub> context); 

    /**
     * @brief 关闭连接池，停止所有阻塞等待，并不再允许获取新的连接
     */
	void close(); 

private:
    std::atomic<bool> b_stop_; ///< 标记连接池是否停止
    size_t pool_size_;         ///< 连接池大小
    std::string host_;         ///< RPC 服务器地址
    std::string port_;         ///< RPC 服务器端口
    std::queue<std::unique_ptr<VertifyService::Stub>> connections_; ///< 连接队列
    std::mutex mutex_;         ///< 保护连接队列的互斥锁
    std::condition_variable cond_; ///< 条件变量，用于等待可用连接
};
#endif