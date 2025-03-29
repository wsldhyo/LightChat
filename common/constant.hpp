#ifndef CONSTANT_HPP
#define CONSTANT_HPP




#define USERIPPREFIX  "uip_"
#define USERTOKENPREFIX  "utoken_"
#define IPCOUNTPREFIX  "ipcount_"
#define USER_BASE_INFO "ubaseinfo_"
#define LOGIN_COUNT  "logincount"

enum class RequestID {
  GET_VERTIFY_CODE,     // 获取验证码
  REGISTER_USER,        // 注册用户
  RESET_PWD,            // 重置密码
  LOGIN_USER,           // 用户登录
  LOGIN_CHAT_SERVER,    // 登录聊天服务器
  LOGIN_CHAT_SERVER_RSP, //登录聊天服务器的回包
};

enum class Modules {
  REGISTER_MOD,
  RESET_PWD_MOD,
  USER_LOGIN_MOD,
};

enum class PwdVisibleState {
  NORMAL,
  SELECTED,
};

enum class ErrorCode {
  NO_ERROR = 0,
  TIP_NO_ERROR,
  TIP_EMAIL_ERR,
  TIP_EMPTY_EMAIL_ERR,
  TIP_PWD_ERR,
  TIP_CONFIRM_ERR,
  TIP_PWD_CONFIRM,
  TIP_VARIFY_ERR,
  TIP_USER_ERR,

  JSON_PARSE_FAILED = 1001, // JSON 解析失败
  NETWORK_ERROR,            // 网络错误
  RPC_FAILED,
  VERTIFY_CODE_EXPIRED,
  VERTIFY_CODE_DISMATCH,
  USER_EXISTED,
  EMAIL_DISMATCH,
  PWD_DISMATCH,
  UPDATE_PWD_FAILED,
  PATH_DO_NOT_EXIST,
  PLATFORM_NOT_SUPPORT,
  PARSE_GATE_PORT_ERROR,
  UID_VALID,
  TOKEN_VALID,

};

constexpr char const *CODE_PREFIX{"code_"};
#endif