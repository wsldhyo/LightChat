#ifndef CLIENT_STRUCT_DEF_HPP
#define CLIENT_STRUCT_DEF_HPP
#include <QString>

struct ServerInfo {
  QString Host;
  QString Port;
  QString Token;
  int Uid;
};
#endif