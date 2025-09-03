#ifndef CONSTANT_HPP
#define CONSTANT_HPP
#include <string_view>
using std::literals::string_view_literals::operator""sv;
// Http请求类型的id
enum class ReqId{
    ID_GET_VERTIFY_CODE = 1001, //获取验证码
    ID_REG_USER = 1002, //注册用户
    ID_RESET_PWD,
    ID_LOGIN_USER,
    ID_CHAT_LOGIN,
    ID_CHAT_LOGIN_RSP,
};

enum class ErrorCodes{
    NO_ERROR = 0,
    PARSE_JSON_FAILED = 1, //Json解析失败
    ERR_NETWORK = 2,     // 网络错误
    RPC_CALL_FAILED = 3, // RPC调用失败
    VERTIFY_CODE_EXPIRED, 
    VERTIFY_CODE_DISMATCH, 
    CONFIRM_PWD_DISMATCH,
    REG_USER_EXISTS,// 注册时，用户名或邮箱已经被占用
    EMAIL_DISMATCH,
    PWD_UPDATE_FAILED,
    PWD_INCORRECT,   // 登录密码不正确
    UID_INVALID,
    TOKEN_INVALID,

    PATH_DO_NOT_EXIST ,
    PLATFORM_NOT_SUPPORT,
    READ_CONFIG_ERROR,    
    OUT_OF_RANGE,
    INVALID_ARGUMENT,

};

// Http请求的模块类型, 每个模块可能有不同的http请求，用ReqId来区分
enum class Modules{
    REGISTERMOD = 0,
    RESETMOD,
    LOGINMOD,
};


constexpr  std::uint16_t TCP_MSG_LEN_MEM_SIZE{2};
constexpr  std::uint16_t TCP_MSG_ID_MEM_SIZE{2};
constexpr  std::uint16_t TCP_MSG_HEAD_MEM_SIZE{TCP_MSG_LEN_MEM_SIZE + TCP_MSG_ID_MEM_SIZE};

constexpr short TCP_MAX_ID{1024};
constexpr short TCP_MAX_SEND_QUE_SIZE{1000};
constexpr short TCP_MAX_RECV_QUE_SIZE{1000};
constexpr short TCP_MAX_LOGIG_QUE_SIZE{1000};

constexpr std::string_view USERTOKENPREFIX  {"utoken_"sv};
constexpr std::string_view IPCOUNTPREFIX  {"ipcount_"sv};

constexpr std::string_view GET_TEST_URL{"/get_test"sv};
constexpr std::string_view POST_GET_VERFIY_CODE{"/get_vertify_code"sv};
constexpr std::string_view POST_REG_USER{"/reg_user"sv};
constexpr std::string_view POST_RESET_PWD{"/reset_pwd"sv};
constexpr std::string_view POST_USER_LOGIN{"/user_login"sv};
constexpr std::string_view VERTIFY_CODE_PREFIX{"code_"sv};

#endif