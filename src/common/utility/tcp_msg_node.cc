#include "tcp_msg_node.hpp"
#include "constant.hpp"
#include <boost/asio.hpp>
// TODO 内存池，避免频繁new delete
// data_多申请一个字节给空字符
TcpMsgNode::TcpMsgNode(std::uint16_t len)
    : data_(new char[len + 1]()), length_(len), cursor_(0) {}

TcpMsgNode::~TcpMsgNode() { delete[] data_; }

TcpMsgNode::TcpMsgNode(TcpMsgNode const &other)
    : data_(new char[other.length_ + 1]()), length_(other.length_),
      cursor_(other.cursor_) {
  std::copy(other.data_, other.data_ + other.length_, data_);
  data_[length_] = '\0';
}

TcpMsgNode &TcpMsgNode::operator=(TcpMsgNode const &other) {
  if (&other == this) {
    return *this;
  }
  delete[] data_;
  data_ = new char[other.length_ + 1]();
  std::copy(other.data_, other.data_ + other.length_, data_);
  data_[other.length_] = '\0';
  length_ = other.length_;
  cursor_ = other.cursor_;
  return *this;
}

TcpMsgNode::TcpMsgNode(TcpMsgNode &&other)
    : data_(std::exchange(other.data_, nullptr)), length_(other.length_),
      cursor_(other.cursor_) {}

TcpMsgNode &TcpMsgNode::operator=(TcpMsgNode &&other) {
  if (this == &other) {
    return *this;
  }
  data_ = std::exchange(other.data_, nullptr);
  length_ = other.length_;
  cursor_ = other.cursor_;
  return *this;
}

void TcpMsgNode::clear() {
  if (data_) {
    std::fill(data_, data_ + length_ + 1, 0);
  }
}

RecvMsgNode::RecvMsgNode(std::uint16_t len, std::uint16_t id)
    : TcpMsgNode(len), msg_id_(id) {}

SendMsgNode::SendMsgNode(std::uint16_t len) : SendMsgNode(nullptr, len, 0) {}

SendMsgNode::SendMsgNode(char const*data, std::uint16_t len, std::uint16_t id)
    : TcpMsgNode(len + TCP_MSG_HEAD_MEM_SIZE), msg_id_(id) {
  // 将id和length转为网络字节序并拷贝到data缓存中
  assign_id(id);
  std::uint16_t net_msg_len =
      boost::asio::detail::socket_ops::host_to_network_short(len);
  char *len_ptr = reinterpret_cast<char *>(&net_msg_len);
  std::copy(len_ptr, len_ptr + sizeof(net_msg_len),
            data_ + TCP_MSG_ID_MEM_SIZE);

  // 拷贝消息体
  if (data) {
    std::copy(data, data + len, data_ + TCP_MSG_HEAD_MEM_SIZE);
    data_[TCP_MSG_HEAD_MEM_SIZE + len] = '\0'; // 修复越界
  }
}

void SendMsgNode::assign_id(std::uint16_t id) {
  if (data_) {
    std::uint16_t net_id_len =
        boost::asio::detail::socket_ops::host_to_network_short(id);
    char *id_ptr = reinterpret_cast<char *>(&net_id_len);
    std::copy(id_ptr, id_ptr + sizeof(net_id_len), data_);
  }
}