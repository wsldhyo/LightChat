#ifndef SERVER_HPP
#define SERVER_HPP
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <memory>


namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace asio = boost::asio;            // from <boost/asio.hpp>
using tcp = asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
/**
 @brief 基于asio和beast实现的Http网关服务器。
        负责接收客户端的http请求，转发给处理该请求的服务器，
        然后将处理结果返回给客户端
*/
class Server : public std::enable_shared_from_this<Server>
{
public:
    Server(asio::io_context& ioc, unsigned short& port);
    void start();
private:
    tcp::acceptor  acceptor_;
    asio::io_context& ioc_;
};

#endif