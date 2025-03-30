#ifndef TCP_MSG_NODE_HPP
#define TCP_MSG_NODE_HPP
struct TcpMsgNode {
  TcpMsgNode(short max_len);

  ~TcpMsgNode();//没用多态，无需虚函数

  void clear();

  short cur_len_;
  short total_len_;
  char *data_;
};

class TcpRecvNode : public TcpMsgNode {
  friend class LogicSystem;

public:
  TcpRecvNode(short max_len, short msg_id);

private:
  short msg_id_;
};

class TcpSendNode : public TcpMsgNode {
  friend class LogicSystem;

public:
  TcpSendNode(const char *msg, short max_len, short msg_id);

private:
  short msg_id_;
};

#endif