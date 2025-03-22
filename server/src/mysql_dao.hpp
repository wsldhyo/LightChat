#ifndef MYSQL_CONNECTION_POOL_HPP
#define MYSQL_CONNECTION_POOL_HPP
#include <condition_variable>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/mysql_driver.h>
#include <memory>
#include <mysql/mysql.h>
#include <queue>
class SqlConnection {
public:
  SqlConnection(sql::Connection *_con, int64_t _last_opetate_time)
      : connection(_con), last_operate_time(_last_opetate_time) {}
  std::unique_ptr<sql::Connection> connection;
  int64_t
      last_operate_time; // 连接mysql后，如果长时间不操作，就断开，让其他请求使用该连接
};

class MySqlPool {
public:
  MySqlPool(const std::string &url, const std::string &user,
            const std::string &pass, const std::string &schema, int poolSize);
  ~MySqlPool();
  std::unique_ptr<SqlConnection> get_connection();
  void return_connection(std::unique_ptr<SqlConnection> _con);
  void close();

private:
  void check_connection();
  std::string url_;    // 连接mysql的url
  std::string user_;   // mysql用户名
  std::string pass_;   // mysql用户密码
  std::string schema_; // 使用的数据库名
  int pool_size_;
  std::queue<std::unique_ptr<SqlConnection>> pool_;
  std::mutex mutex_;
  std::condition_variable cond_;
  std::atomic<bool> b_stop_;
  std::thread
      check_thread_; // 定时向mysql发起简单的请求，告诉mysql连接存活，相当于心跳机制
};


struct UserInfo {
	std::string name;
	std::string pwd;
	int uid;
	std::string email;
};

class MysqlDao
{
public:
	MysqlDao();
	~MysqlDao();
	int register_user(std::string const& _name, std::string const& _email, std::string const& _pwd);
	int register_user(std::string const& _name, std::string const& _email, std::string const& _pwd, std::string const& _icon );
	bool check_email(std::string const& _name, std::string const & _email);
	bool update_pwd(std::string const& _name, std::string const& _newpwd);
	bool check_pwd(std::string const& _name, std::string const& _pwd, UserInfo& _userInfo);
private:
	std::unique_ptr<MySqlPool> pool_;
};

#endif