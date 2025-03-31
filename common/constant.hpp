#ifndef CONSTANT_HPP
#define CONSTANT_HPP

#define USERIPPREFIX "uip_"
#define USERTOKENPREFIX "utoken_"
#define IPCOUNTPREFIX "ipcount_"
#define USER_BASE_INFO "ubaseinfo_"
#define LOGIN_COUNT "logincount"

enum class RequestID {
  GET_VERTIFY_CODE, // 获取验证码
  REGISTER_USER,    // 注册用户
  RESET_PWD,        // 重置密码
  // 客户端先向GateServer发送登录请求，GateSever验证通过后
  // 客户端再发送LOGIN_CHAT_SERVER请求来登录聊天服务器
  LOGIN_USER,            // 用户登录，
  LOGIN_CHAT_SERVER,     // 登录聊天服务器
  LOGIN_CHAT_SERVER_RSP, // 登录聊天服务器的回包
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

enum class ChatUiMode {
  SEARCH_MODE,   // 搜索模式
  CHAT_MODE,     // 聊天记录搜索模式
  CONTRACT_MODE, // 联系人搜索模式 ,
};

enum class ListItemType {
  CHAT_USER_ITEM,    // 聊天用户
  CONTACT_USER_ITEM, // 联系人用户
  SEARCH_USER_ITEM,  // 搜索到的用户
  ADD_USER_TIP_ITEM, // 提示添加用户
  INVALID_ITEM,      // 不可点击条目
  GROUP_TIP_ITEM,    // 分组提示条目
  LINE_ITEM,         // 分割线
  APPLY_FRIEND_ITEM, // 好友申请
};

constexpr char const *CODE_PREFIX{"code_"};

constexpr short TCP_MSG_ID_MEM_LENGTH{2};
constexpr short TCP_MSG_BODY_MEM_LENGTH{2};
constexpr short TCP_MSG_HEAD_MEM_LENGTH{TCP_MSG_ID_MEM_LENGTH +
                                        TCP_MSG_BODY_MEM_LENGTH};

constexpr short TCP_MAX_MSG_LENGTH{2 * 1024};
constexpr short TCP_MAX_ID_LENGTH{1024};
constexpr short TCP_MAX_SEND_QUE_SIZE{1000};
constexpr short TCP_MAX_RECV_QUE_SIZE{1000};
constexpr short TCP_MAX_LOGIG_QUE_SIZE{1000};

#endif