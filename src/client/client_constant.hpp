#ifndef CLIENT_CONSTANT_HPP
#define CLIENT_CONSTANT_HPP
#include <QString>

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

/**
 * @brief 聊天界面的显示模式
 *
 */
enum class ChatUIMode {
  SEARCH_MODE,  // 搜索模式
  CHAT_MODE,    // 聊天模式
  CONTACT_MODE, // 联系人模式
};

/**
 * @brief 自定义QListWidgetItem的几种类型
 *
 */
enum class ListItemType {
  CHAT_USER_ITEM,    //聊天用户
  CONTACT_USER_ITEM, //联系人用户
  SEARCH_USER_ITEM,  //搜索到的用户
  ADD_USER_TIP_ITEM, //提示添加用户
  INVALID_ITEM,      //不可点击条目
  GROUP_TIP_ITEM,    //分组提示条目
  LINE_ITEM,         //分割线
  APPLY_FRIEND_ITEM, //好友申请
};

enum class ChatRole {
  SELF,
  OTHERS,
};

constexpr int BUBBLE_TRIANGLE_WIDTH{8}; //三角宽
constexpr int PIC_MAX_WIDTH{160};
constexpr int PIC_MAX_HEIGHT{90};

// 申请好友框的最小长度
constexpr int MIN_APPLY_LABEL_ED_LEN{40};
// 添加好友对话框中的标签偏移量
constexpr int g_tip_offset{5};

const QString g_add_prefix{QStringLiteral(u"添加标签")};
#endif
