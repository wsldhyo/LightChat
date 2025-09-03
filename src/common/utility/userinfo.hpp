#ifndef USER_INFO_HPP
#define USER_INFO_HPP
#include <string>
struct UserInfo {
  std::string name;
  std::string pwd;
  int uid;
  std::string email;
};
#endif