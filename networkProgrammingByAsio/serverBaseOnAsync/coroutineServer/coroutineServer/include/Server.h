#pragma once
#include <string>
#include <boost/asio.hpp>
#include <map>
#include <memory>
#include <mutex>
#include "../include/Session.h"
#include "../include/AsioIOServicePool.h"

class Session;

class Server {
public:
    Server(boost::asio::io_context&,short);
    void clearSession(std::string);
    ~Server();
private:
    void HandleAccept(std::shared_ptr<Session> new_session,const boost::system::error_code& ec);
    void startAccept();
    boost::asio::io_context& _ioc;
    short _port;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::map<std::string,std::shared_ptr<Session>> _sessions;
    std::mutex _mutex;
};