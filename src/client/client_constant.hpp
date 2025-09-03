#ifndef CLIENT_CONSTANT_HPP
#define CLIENT_CONSTANT_HPP

// 注册界面错误消息提示类型
enum TipErr {
  TIP_SUCCESS = 0,
  TIP_EMAIL_ERR = 1,
  TIP_PWD_ERR = 2,
  TIP_CONFIRM_ERR = 3,
  TIP_PWD_CONFIRM = 4,
  TIP_VERTIFY_ERR = 5,
  TIP_USER_ERR = 6
};

// 可点击按钮的显示状态
enum ClickLbState { Normal = 0, Selected = 1 };

#endif