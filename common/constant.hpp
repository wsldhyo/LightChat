#ifndef CONSTANT_HPP
#define CONSTANT_HPP

enum class RequestID {
  GET_VERTIFY_CODE,
  REGISTER_USER,
};

enum class Modules {
  REGISTER_MOD,
};

enum class PwdVisibleState {
  NORMAL,
  SELECTED,
};

enum class ErrorCode {
  NO_ERROR = 0,
  TIP_NO_ERROR,
  TIP_EMAIL_ERR,
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
  PATH_DO_NOT_EXIST,
  PLATFORM_NOT_SUPPORT,
  PARSE_GATE_PORT_ERROR,
};

constexpr char const *CODE_PREFIX{"code_"};
#endif