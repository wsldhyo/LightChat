#ifndef USER_INFO_HPP
#define USER_INFO_HPP
#include <string>

struct UserInfo {
    std::string name;  // 用户名（登录名或账号名）
    std::string pwd;   // 用户密码（通常存储加密/哈希后的密码）
    int uid;           // 用户唯一ID（数据库主键或全局唯一标识符）
    std::string email; // 用户邮箱
    std::string nick;  // 用户昵称（显示给其他用户的名字）
    std::string desc;  // 用户描述/签名/个人简介
    int sex;           // 性别（常用0/1/2表示未知/男/女，具体需看业务定义）
    std::string icon;  // 用户头像URL或文件名
    std::string back;  // 用户背景图片URL或文件名（个人主页背景等）
};


struct ApplyInfo {
	ApplyInfo(int uid, std::string name, std::string desc,
		std::string icon, std::string nick, int sex, int status)
		:_uid(uid),_name(name),_desc(desc),
		_icon(icon),_nick(nick),_sex(sex),_status(status){}

	int _uid;
	std::string _name;
	std::string _desc;
	std::string _icon;
	std::string _nick;
	int _sex;
	int _status;
};


#endif