#ifndef CONSTANT_HPP
#define CONSTANT_HPP

enum class RequestID {
  GET_VERTIFY_CODE,
  REGISTER_USER,
};

enum class Modules {
  REGISTER_MOD,
};

enum class ErrorCode {
  NO_ERROR = 0,
  JSON_PARSE_FAILED = 1001, // JSON 解析失败
  NETWORK_ERROR,            // 网络错误
  RPC_FAILED,
  PATH_DO_NOT_EXIST,
  PLATFORM_NOT_SUPPORT,
  PARSE_GATE_PORT_ERROR,
};
#endif