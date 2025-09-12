#ifndef USER_MANAGER_HPP
#define USER_MANAGER_HPP
#include "utility/singleton.hpp"
#include <unordered_map>
class Session;
class UserManager : public Singleton<UserManager>
{
	friend class Singleton<UserManager>;
public:
	~UserManager();
	std::shared_ptr<Session> get_session(int uid);
	void set_user_session(int uid, std::shared_ptr<Session> session);
	void remove_user_session(int uid);

private:
	UserManager();
	std::mutex session_mtx_;
	std::unordered_map<int, std::shared_ptr<Session>> uid_to_session_;
};

#endif // USER_MANAGER_HPP