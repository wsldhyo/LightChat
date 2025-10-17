#ifndef LOGIC_SYSTEM_HPP
#define LOGIC_SYSTEM_HPP
#include "utility/constant.hpp"
#include "utility/singleton.hpp"
#include "utility/tcp_msg_node.hpp"
#include <condition_variable>
#include <functional>
#include <json/value.h>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <string_view>
#include <thread>
class Session;
class Server;
class UserInfo;
class ApplyInfo;
/**
 * @brief 一个轻量级的 Lambda/函数包装器，用于存储消息处理回调。
 *
 * 与 std::function 不同，CallbackRef 避免了类型擦除带来的额外开销，
 * 使用手动管理的 unique_ptr 保存具体的函数对象，并通过函数指针调度。
 *
 * - 构造时：捕获传入的可调用对象并包装
 * - 调用时：通过 fn 调用存储的具体 Functor
 *
 * @note 不可拷贝，仅支持移动。
 */
struct CallbackRef : public NonCopyable {
  // 函数指针类型，签名 为
  //        void function(void* obj, std::shared_ptr<Session> session, ReqId id,
  //        std::string_view msg);
  using Fn = void (*)(void *, std::shared_ptr<Session>, ReqId,
                      std::string_view msg);

  /// 待删除器的unique_ptr，初始删除器什么也不做。构造函数里替换为真正的删除器
  std::unique_ptr<void, void (*)(void *)> obj{nullptr, [](void *) {}};

  /// 实际的函数调度器（调用时会把 obj 转换回具体的 Functor*）
  Fn fn = nullptr;

  /**
   * @brief 构造函数，包装任意可调用对象。
   *
   * @tparam F  可调用对象类型（lambda、仿函数、函数指针等）
   * @param f   可调用对象
   *
   * 实现：
   * 1. new 出一个 Functor（存入 obj）
   * 2. 设置 fn（通过 void* 强转回 Functor* 并执行）
   */
  template <typename F> CallbackRef(F &&f) {
    using Functor = std::decay_t<F>;
    auto ptr = new Functor(std::forward<F>(f));
    // lambda作为删除器，无捕获lambda自动退化为函数指针
    obj = {ptr, [](void *p) { delete static_cast<Functor *>(p); }};
    // 无捕获lambda自动退化为函数指针
    // 内部调用Funtor(s, id, msg)
    fn = [](void *o, std::shared_ptr<Session> s, ReqId id,
            std::string_view msg) { (*static_cast<Functor *>(o))(s, id, msg); };
  }

  /**
   * @brief 调用存储的回调。
   *
   * @param s   会话对象
   * @param id  请求 ID
   * @param msg 消息内容
   *
   * 实现：转发给 fn(obj, s, id, msg)，实际由存储的 Functor 执行。
   */
  void operator()(std::shared_ptr<Session> s, ReqId id,
                  std::string_view msg) const {
    fn(obj.get(), s, id, msg);
  }
  CallbackRef(CallbackRef &&) noexcept = default;
  CallbackRef &operator=(CallbackRef &&) noexcept = default;
};

/**
 * @brief LogicSystem 内部的消息节点。
 *
 * 用于把网络层收到的消息包装成逻辑层任务，
 * 携带消息数据（RecvMsgNode）和来源会话（Session）。
 */
struct LogicNode {
  std::unique_ptr<RecvMsgNode> msg_;
  std::shared_ptr<Session> session_;
};

/**
 * @brief 聊天服务器的逻辑处理系统（单例）。
 *
 * 功能：
 * - 接收网络线程投递的消息（post_msg）
 * - 将消息分发给对应的业务逻辑回调（handlers_）
 * - 维护用户信息（users_）
 *
 * - 内部维护一个工作线程（work_thread_）负责消息消费
 * - 使用条件变量 + 消息队列实现生产者/消费者模型
 *
 * 线程安全：
 * - post_msg() 可被多线程调用（来自网络 IO 线程），内部加锁保护
 * - deal_msg() 在单独线程运行，不与 post_msg 并发操作队列
 */
class LogicSystem : public Singleton<LogicSystem> {
  friend class Singleton<LogicSystem>;
  LogicSystem();

public:
  /**
   * @brief 析构函数
   *
   * - 通知工作线程退出
   * - 等待工作线程 join
   */
  ~LogicSystem();

  /**
   * @brief 向逻辑系统投递一条消息。
   *
   * 由网络层调用（Session::on_message），把收到的消息交给逻辑层处理。
   *
   * @param msg     网络层解析出的消息节点（RecvMsgNode）
   * @param session 消息来源的会话指针
   *
   * 流程：
   * - 将消息包装成 LogicNode 并压入队列
   * - 如果队列从空变为非空，唤醒工作线程
   *
   * @note 有最大队列长度限制（TCP_MAX_LOGIG_QUE_SIZE），超过则丢弃消息。
   */
  void post_msg(std::unique_ptr<RecvMsgNode> msg,
                std::shared_ptr<Session> session);

  void set_server(std::shared_ptr<Server> server);

private:
  /**
   * @brief 逻辑处理线程的主循环。
   * - 等待消息队列非空（条件变量唤醒）
   * - 批量取出消息（避免长时间持锁）
   * - 遍历消息，根据 msg_id 查找对应的回调
   * - 调用回调处理业务逻辑
   *
   */
  void deal_msg();

  /**
   * @brief 注册所有消息处理回调。
   *
   */

  void register_msg_handler();
  /**
   * @brief 构造函数
   *
   * - 注册所有消息处理回调（register_msg_handler）
   * - 启动逻辑处理线程（deal_msg）
   */

  bool get_base_info(std::string const &base_key, int uid, UserInfo &userinfo);
  /**
   * @brief 根据用户 UID 查询用户信息
   *
   * 优先从 Redis 缓存中获取用户基本信息；
   * 如果 Redis 中没有或解析失败，则从 MySQL 数据库查询；
   * 并在查询成功后，将结果写回 Redis 做缓存。
   *
   * @param uid_str  字符串形式的用户 UID（用于构建 Redis key）
   * @param uid      整型用户 UID（用于数据库查询）
   * @param ret_value [out] JSON 对象，包含用户信息及 error 字段
   *
   * @note 返回结果ret_value中始终包含 "error" 字段：
   *       - 0：成功
   *       - PARSE_JSON_FAILED：Redis 中 JSON 解析失败
   *       - UID_INVALID：数据库中未找到对应用户
   */
  void get_user_by_uid(std::string const &uid_str, int32_t uid,
                       Json::Value &ret_value);

  /**
   * @brief 根据用户名查询用户信息
   *
   * 优先从 Redis 缓存中获取用户基本信息；
   * 如果 Redis 中没有或解析失败，则从 MySQL 数据库查询；
   * 并在查询成功后，将结果写回 Redis 做缓存。
   *
   * @param name     用户名（用于 Redis key 和数据库查询）
   * @param rtvalue  [out] JSON 对象，包含用户信息及 error 字段
   *
   * @note 返回结果始终包含 "error" 字段：
   *       - 0：成功
   *       - PARSE_JSON_FAILED：Redis 中 JSON 解析失败
   *       - UID_INVALID：数据库中未找到对应用户
   */
  void get_user_by_name(std::string const &name, Json::Value &rtvalue) const;

  /**
   * @brief 获取指定用户的好友申请列表，并序列化为 JSON 格式返回
   *
   * 该函数会从数据库中查询某个用户收到的好友申请列表，将结果存储到
   * `apply_list` 中，再逐条转换为 JSON 对象，组装成 JSON 数组，最后存放到
   * `rtvalue["apply_list"]` 中。
   *
   * @param to_uid   接收好友申请的用户 ID
   * @param rtvalue  返回结果 JSON，增加字段
   * "apply_list"，其中包含好友申请信息数组
   *
   * @note
   * - 内部调用 MysqlMgr::get_apply_list 从数据库获取数据；
   * - 如果查询失败或没有数据，`rtvalue` 不会包含 "apply_list" 字段；
   */
  void get_friend_apply_info(int const to_uid, Json::Value &rtvalue) const;

  /**
   * @brief 向数据库查询用户的好友列表，以转发给客户端展示
   *
   * @param self_id 本用户id
   * @param user_list 存储查询到的好友列表
   * @return true
   *    成功查询到好友列表
   * @return false
   *    查询好友列表失败
   */
  bool get_friend_list(int self_id, Json::Value &rtvalue);
  std::mutex msg_que_mutex_;
  bool b_stop_;
  std::condition_variable deal_msg_cond_;
  std::queue<LogicNode> recv_msg_que_;
  std::unordered_map<ReqId, CallbackRef> handlers_;
  std::thread work_thread_;
  std::shared_ptr<Server> server_;
};

#endif