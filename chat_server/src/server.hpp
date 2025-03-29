#ifndef SERVER_HPP
#define SERVER_HPP
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <string>
#include <unordered_map>
#include <memory>
#include <mutex>
class Session;
class Server{
public:
    Server(boost::asio::io_context& _ioc, int _port);
    void remove_session(std::string const& _uuid);

private:
    void start_accept();
    boost::asio::io_context& ioc_;
    int port_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;
    std::mutex mutex_;
};
#endif