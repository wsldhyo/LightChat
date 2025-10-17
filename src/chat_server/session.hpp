#ifndef SESSION_HPP
#define SESSION_HPP
#include "utility/tcp_msg_node.hpp"
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/read.hpp>
#include <iostream>
#include <memory>
#include <queue>
#include <string>
using boost::asio::ip::tcp;
class Server;

// 读回调类型约束
template <typename Handler>
concept ReadHandler = requires(Handler h, boost::system::error_code ec,
                               std::size_t n) {
  { h(ec, n) } -> std::same_as<void>;
};
/**

C++20 前可以使用SFINAE，如下

// 检查 Handler 是否可调用，且返回 void
template <typename Handler, typename = void>
struct is_read_handler : std::false_type {};

template <typename Handler>
struct is_read_handler<
    Handler,
    std::void_t<decltype(std::declval<Handler>()(std::declval<error_code>(),
std::declval<std::size_t>()))> > : std::is_same<void,
decltype(std::declval<Handler>()(std::declval<error_code>(),
std::declval<std::size_t>()))> {};

// 异步函数模板
template <typename Handler,
          typename = std::enable_if_t<is_read_handler<Handler>::value>>
void async_read_some(Handler&& handler) {
    handler(error_code{}, 42);
}

*/

/**
 * @brief 表示一个 TCP 会话（Session），负责处理客户端与服务端之间的数据交互。
 *
 * 该类封装了：
 * - 异步读取消息头与消息体
 * - 异步发送消息队列
 * - 会话生命周期管理（自动关闭、通知 Server 移除）
 *
 * 使用 shared_ptr 管理自身生命周期，避免异步回调中悬空。
 */
class Session : public std::enable_shared_from_this<Session> {
public:
  /**
   * @brief 构造函数
   * @param peer 已经建立连接的 TCP socket
   * @param server 指向所属的 Server 实例（弱引用，避免循环依赖）
   */
  Session(tcp::socket peer, std::shared_ptr<Server> server);

  /**
   * @brief 析构函数，确保会话关闭
   */
  ~Session();

  /**
   * @brief 启动会话，开启异步读取流程
   */
  void start();

  /**
   * @brief 获取该会话的唯一标识符（UUID）
   * @return 会话 UUID 字符串
   */
  std::string const &get_session_id() const;

  void set_user_id(int32_t uid);

  int32_t get_user_id() const;
  /**
   * @brief 发送字符串消息,
   * @param msg 消息内容
   * @param msg_id 消息 ID（应用层协议定义）
   */
  void send(char const *msg, std::size_t msg_len, std::uint16_t msg_id);

  /**
   * @brief 发送二进制消息
   * @param msg 消息数据指针
   * @param msg_len 消息长度
   * @param msg_id 消息 ID（应用层协议定义）
   */
  void send(std::string const &msg, std::uint16_t msg_id);

  /**
   * @brief 关闭会话（幂等操作）
   *
   * - 关闭 socket
   * - 通知 Server 移除该 Session
   */
  void close();

  void notify_offline(int32_t uid);

private:
  /**
   * @brief 异步读取指定长度的数据，直到读满或发生错误
   * @tparam Handler 回调类型，需满足 `void(boost::system::error_code,
   * std::size_t)`
   * @param buf 目标缓冲区
   * @param buf_size 期望读取的总字节数
   * @param already_read_len 已读取的字节数（支持断点续读）
   * @param handler 读取完成或出错后的回调函数
   *
   * - 正常完成时，回调传递 `(error_code{}, 已读取字节数)`
   * - 出错时，回调传递 `(错误码, 已读取字节数)`
   */
  template <typename Handler>
  void async_read_len(char *buf, std::size_t buf_size,
                      std::size_t already_read_len, Handler &&handler) {
    // 参数检查省略（和你一致）
    if (!buf) {
      handler(boost::asio::error::invalid_argument, already_read_len);
      return;
    }
    if (already_read_len > buf_size) {
      handler(boost::asio::error::invalid_argument, already_read_len);
      return;
    }
    if (buf_size == 0 || already_read_len == buf_size) {
      handler(boost::system::error_code{}, already_read_len);
      return;
    }

    using HandlerT = std::decay_t<Handler>;

    // 异步读取的状态类
    struct ReadOp : std::enable_shared_from_this<ReadOp> {
      char *buf;
      std::size_t buf_size;
      std::size_t already_read_len;
      HandlerT handler; // 存放用户 handler（decay 后）
      std::shared_ptr<Session> self;

      ReadOp(char *b, std::size_t bs, std::size_t al, HandlerT &&h,
             std::shared_ptr<Session> s)
          : buf(b), buf_size(bs), already_read_len(al),
            handler(std::forward<HandlerT>(h)), self(std::move(s)) {}

      void operator()(const boost::system::error_code &ec,
                      std::size_t bytes_transferred) {
        if (ec) {
          handler(ec, already_read_len);
          return;
        }

        already_read_len += bytes_transferred;
        if (already_read_len < buf_size) {
          auto buffer = boost::asio::buffer(buf + already_read_len,
                                            buf_size - already_read_len);

          // 继续自回调ReadOp::operator()()
          auto op = this->shared_from_this();
          self->peer_.async_read_some(buffer,
                                      [op](const boost::system::error_code &ec,
                                           std::size_t n) { (*op)(ec, n); });
        } else {
          handler({}, already_read_len);
        }
      }
    };

    // 创建第一个操作对象；注意这里用的是 std::forward<Handler>
    auto op = std::make_shared<ReadOp>(buf, buf_size, already_read_len,
                                       std::forward<HandlerT>(handler),
                                       shared_from_this());

    auto buffer = boost::asio::buffer(buf + already_read_len,
                                      buf_size - already_read_len);

    // 初次调用也用 lambda 包装，确保 op 被持有
    peer_.async_read_some(buffer, [op](const boost::system::error_code &ec,
                                       std::size_t n) { (*op)(ec, n); });
  }

  /**
   * @brief 消息头读取完成后的回调函数
   * @param ec 错误码
   * @param bytes_transferred 实际读取的字节数
   *
   * - 若成功，解析消息头并启动消息体读取
   * - 若失败，关闭会话
   */
  void readhead_callback(boost::system::error_code ec,
                         std::size_t bytes_transferred);

  /**
   * @brief 消息体读取完成后的回调函数
   * @param ec 错误码
   * @param bytes_transferred 实际读取的字节数
   *
   * - 若成功，将完整消息投递给 LogicSystem
   * - 若失败，关闭会话
   */
  void readbody_callback(boost::system::error_code ec,
                         std::size_t bytes_transferred);

  /**
   * @brief 发送完成后的回调函数
   * @param ec 错误码
   * @param bytes_transferred 实际发送的字节数
   *
   * - 若成功，继续发送队列中的下一条消息
   * - 若失败，关闭会话
   */
  void send_callback(boost::system::error_code ec,
                     std::size_t bytes_transferred);
  /**
   * @brief 处理未使用或异常会话的清理工作
   *
   * 当会话异常关闭、客户端断开、心跳超时或者其他情况下，
   * 需要清理与该 session 相关的资源，包括：
   * 1. 从 Redis 中删除用户 session 信息
   * 2. 删除用户登录信息
   * 3. 通知 Server 从内存 map 中移除该 session
   *
   * 该函数会先尝试获取 Redis 分布式锁，确保在多节点环境下安全操作，
   * 避免误删除其他服务器上活跃的 session。
   *
   * 锁释放和 Server 移除 session 的操作通过 Defer 机制保证即使中途 return
   * 也能执行。
   */
  void deal_unused_session();

private:
  tcp::socket peer_;             ///< 与客户端通信的 socket
  std::weak_ptr<Server> server_; ///< 关联的服务器实例（弱引用）
  std::string session_id_;       ///< 会话唯一 ID（UUID）
  std::unique_ptr<TcpMsgNode> recv_head_node_;        ///< 消息头缓冲区
  std::unique_ptr<RecvMsgNode> recv_msg_node_;        ///< 消息体缓冲区
  std::mutex send_que_mutex_;                         ///< 发送队列互斥锁
  std::queue<std::unique_ptr<SendMsgNode>> send_que_; ///< 发送队列
  std::atomic_bool closing_{false}; ///< 会话是否正在关闭（幂等保护）
  int32_t user_id_;
};
#endif
