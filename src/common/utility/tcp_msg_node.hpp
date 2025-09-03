#ifndef TCP_MSG_NODE_HPP
#define TCP_MSG_NODE_HPP
#include <cstdint>
struct TcpMsgNode {
    explicit TcpMsgNode(std::uint16_t len);
    TcpMsgNode(TcpMsgNode const& other);
    TcpMsgNode& operator=(TcpMsgNode const& other);
    TcpMsgNode(TcpMsgNode && other);
    TcpMsgNode& operator=(TcpMsgNode && other);
    ~TcpMsgNode();
    void clear();
    char* data_;
    std::uint16_t length_;
    std::uint16_t cursor_;

};

struct RecvMsgNode : public TcpMsgNode
{
    RecvMsgNode(std::uint16_t len, std::uint16_t id);
    ~RecvMsgNode() = default;
    std::uint16_t msg_id_;
};

struct SendMsgNode : public TcpMsgNode
{
    explicit SendMsgNode(std::uint16_t len);
    SendMsgNode(char const* data, std::uint16_t len, std::uint16_t id);
    ~SendMsgNode() = default;
    void assign_id(std::uint16_t id);
    std::uint16_t msg_id_;
};
#endif