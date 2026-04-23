#pragma once
#include <iostream>
#include "boost/asio.hpp"
#include <memory>
#include <map>
#include "session_v3.h"
namespace asio = boost::asio;
namespace ip = asio::ip;
using tcp = ip::tcp;

class Session;

class Server{
public:
    Server(asio::io_context& io,unsigned short port_num);
    void clearSession(std::string uuid){
        _sessions.erase(uuid); //若uuid不存在，则无事发生
    }

private:
    void start_Accept();
    void handle_Accept(std::shared_ptr<Session> new_session,const boost::system::error_code& ec);//接收连接的回调函数
    asio::io_context& _ioc;
    tcp::acceptor _acceptor;
    std::map<std::string,std::shared_ptr<Session>> _sessions; //使用map来管理会话，有连接就插入，断开连接就删除
};