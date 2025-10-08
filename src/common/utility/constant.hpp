#ifndef CONSTANT_HPP
#define CONSTANT_HPP
#include <string_view>
using std::literals::string_view_literals::operator""sv;
// Http请求类型的id
enum class ReqId {
  ID_GET_VERTIFY_CODE = 1001, //获取验证码
  ID_REG_USER,                //注册用户
  ID_RESET_PWD,               //重置密码
  ID_LOGIN_USER, // 用户登录请求，状态服务器分配聊天服务器给客户端
  ID_CHAT_LOGIN_REQ, // 聊天服务器登录请求
  ID_CHAT_LOGIN_RSP, // 聊天服务器登录应，客户端将切换到聊天界面
  ID_SEARCH_USER_REQ, // 用户查询请求
  ID_SEARCH_USER_RSP, // 用户查询响应，客户端将展示服务端返回的查询结果
  ID_APPLY_FRIEND_REQ,        // 申请添加好友
  ID_NOTIFY_FRIEND_APPLY_REQ, // 通知对方有新好友申请
  ID_APPLY_FRIEND_RSP, // 申请添加好友的服务器处理回包（是否转发申请给被申请方或者有错误发生）
  ID_AUTH_FRIEND_REQ,  // 被申请方的认证好友请求，服务器转发认证结果给好友申请方
  ID_AUTH_FRIEND_RSP,  // 被申请方客户端根据服务器回包，可将对方显示到联系人列表（若同意成为好友）
  ID_NOTIFY_AUTH_FRIEND_REQ, // 通知申请方0好友认证结果，对方客户端据此可将对方显示到联系人列表（若同意成为好友）
};

enum class ErrorCodes {
  NO_ERROR = 0,
  PARSE_JSON_FAILED = 1, // Json解析失败
  ERR_NETWORK = 2,       // 网络错误
  RPC_CALL_FAILED = 3,   // RPC调用失败
  VERTIFY_CODE_EXPIRED,  // 验证码过期
  VERTIFY_CODE_DISMATCH, // 验证码错误（和服务器缓存的不一致）
  CONFIRM_PWD_DISMATCH,  // 确认密码和密码输入不一致
  REG_USER_EXISTS,       // 注册时，用户名或邮箱已经被占用
  EMAIL_DISMATCH,        // 邮箱和用户名不匹配
  PWD_UPDATE_FAILED,     // 密码更新失败
  PWD_INCORRECT,         // 登录密码不正确
  UID_INVALID,           // UID无效
  TOKEN_INVALID, // 令牌无效，令牌可用于聊天服务器登录的验证

  PATH_DO_NOT_EXIST,    // 文件路径不存在
  PLATFORM_NOT_SUPPORT, // 不支持的平台
  READ_CONFIG_ERROR,    // 读取配置文件错误
  OUT_OF_RANGE,         //字符串转换后的数字超出范围
  INVALID_ARGUMENT,     //字符串转数字失败，无效参数

};

// Http请求的模块类型, 每个模块可能有不同的http请求，用ReqId来区分
enum class Modules {
  REGISTERMOD = 0,
  RESETMOD,
  LOGINMOD,

};

// TCP消息节点头部信息
constexpr std::uint16_t TCP_MSG_LEN_MEM_SIZE{2};
constexpr std::uint16_t TCP_MSG_ID_MEM_SIZE{2};
constexpr std::uint16_t TCP_MSG_HEAD_MEM_SIZE{TCP_MSG_LEN_MEM_SIZE +
                                              TCP_MSG_ID_MEM_SIZE};
// TCP连接的队列信息
constexpr short TCP_MAX_ID{1024};
constexpr short TCP_MAX_SEND_QUE_SIZE{1000};
constexpr short TCP_MAX_RECV_QUE_SIZE{1000};
constexpr short TCP_MAX_LOGIG_QUE_SIZE{1000};

// Redis键
// 和uid拼接，查找登录的token
constexpr std::string_view REDIS_USER_TOKEN_PREFIX{"utoken_"sv};
constexpr std::string_view REDIS_IP_COUNT_PREFIX{"ipcount_"sv};
//和邮箱拼接， 查找发给对应邮箱的验证码
constexpr std::string_view REDIS_VERTIFY_CODE_PREFIX{"code_"sv};
// 和uid拼接，查找对应uid登录的聊天服务器名字
constexpr std::string_view REDIS_USER_IP_PREFIX{"uip_"sv};
// 和uid拼接，查询用户的详细信息（UserInfo中的全部字段）
constexpr std::string_view REDIS_USER_BASE_INFO_PREFIX{"ubaseinfo_"sv};
// 和用户名拼接，查询用户的详细信息（UserInfo中的全部字段）
constexpr std::string_view REDIS_NAME_INFO_PREFIX{"nameinfo_"sv};
// 查询每台服务器当前登录用户数目
constexpr std::string_view REDIS_LOGIN_COUNT_PREFIX{"logincount"sv};

// http请求路由
constexpr std::string_view GET_TEST_URL{"/get_test"sv};
constexpr std::string_view POST_GET_VERFIY_CODE{"/get_vertify_code"sv};
constexpr std::string_view POST_REG_USER{"/reg_user"sv};
constexpr std::string_view POST_RESET_PWD{"/reset_pwd"sv};
constexpr std::string_view POST_USER_LOGIN{"/user_login"sv};

#endif