#ifndef REDIS_MANAGER_HPP
#define REDIS_MANAGER_HPP
#include "../../common/singleton.hpp"
#include <hiredis/hiredis.h>

class RedisConnectPool;
class RedisConnectionManager : public Singleton<RedisConnectionManager>
{
    friend class Singleton<RedisConnectionManager>;
    public:
    ~RedisConnectionManager();

    /**
     * @brief 通过hiredis的redisCommand获取从redis数据库获取key
     * 对应的valu
     * @param key 
     * @param [out] value  获取到的value
     * @return true 
     * @return false 
     */
    bool get(const std::string &key, std::string& value);

    /**
     * @brief 通过hiredis的redisCommand接口设置key在数据库中的值
     * 
     * @param key 
     * @param value 
     * @return true 
     * @return false 
     */
    bool set(const std::string &key, const std::string &value);

    /**
     * @brief 通过hiredis的redisCommand进行redis服务认证，redis服务可能设置了认证密码
     * 
     * @param password 认证密码
     * @return true 
     * @return false 
     */
    bool authorize(const std::string &password);
    
    /**
     * @brief 将key和value插入到redis底层链表最左侧
     *  
     * @param key 
     * @param value 
     * @return true 
     * @return false 
     */
    bool l_push(const std::string &key, const std::string &value);
   
    /**
     * @brief 从redis的底层链表获取最左边的键值对
     * 
     * @param key 获取到的链表最左边的键 
     * @param value 获取到的最左边的键对应的值
     * @return true 
     * @return false 
     */
    bool l_pop(const std::string &key, std::string& value);
   
    bool r_push(const std::string& key, const std::string& value);
    
    bool r_pop(const std::string& key, std::string& value);
    
    /**
     * @brief  hset用于操作哈希表，一个键对应一个哈希表，这个哈希表内可以具有多个字段。
     *    hset适合结构化数据存储，例如hset user:1001 name:"Alice" age:"25" gender"female"
     *    这样， user:1001是键，后面的键值对都存储在一个哈希表
     *    所有这样的键又组合成了一个哈希表。所以hset相当于操作一个二级哈希表
     * @param key 哈希表主键
     * @param hkey 字段名 
     * @param value 字段对应的值
     * @return true 
     * @return false 
     */
    bool h_set(const std::string &key, const std::string  &hkey, const std::string &value);
    
    bool h_set(const char* key, const char* hkey, const char* hvalue, size_t hvaluelen);
    
    bool h_get(const std::string &key, const std::string &hkey, std::string& _value);
    
    bool del(const std::string &key);
    
    bool exists_key(const std::string &key);
    
    void close();
private:
    RedisConnectionManager();

    std::unique_ptr<RedisConnectPool> pool_;
};
#endif