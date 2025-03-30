#include <cstring>
#include <boost/asio/detail/socket_ops.hpp>

#include "tcp_msg_node.hpp"
#include "constant.hpp"

TcpMsgNode::TcpMsgNode(short _max_len) : total_len_(_max_len), cur_len_(0) {
  data_ = new char[_max_len + 1]();
  data_[_max_len] = '\0';
}

TcpMsgNode::~TcpMsgNode() { delete[] data_; }

void TcpMsgNode::clear() {
  memset(data_, 0, total_len_ + 1);
  cur_len_ = 0;
}

TcpRecvNode::TcpRecvNode(short _max_len, short _msg_id)
    : TcpMsgNode(_max_len), msg_id_(_msg_id) {}

TcpSendNode::TcpSendNode(const char *msg, short _max_len, short _msg_id)
    : TcpMsgNode(TCP_MSG_HEAD_MEM_LENGTH), msg_id_(_msg_id) {
  // 先发送id, 转为网络字节序
  short msg_id_host =
      boost::asio::detail::socket_ops::host_to_network_short(msg_id_);
  memcpy(data_, &msg_id_host, TCP_MSG_ID_MEM_LENGTH);
  // 转为网络字节序
  short max_len_host =
      boost::asio::detail::socket_ops::host_to_network_short(_max_len);
  memcpy(data_ + TCP_MSG_ID_MEM_LENGTH, &max_len_host, TCP_MSG_BODY_MEM_LENGTH);
  memcpy(data_ + TCP_MSG_ID_MEM_LENGTH + TCP_MSG_BODY_MEM_LENGTH, msg, _max_len);
}
