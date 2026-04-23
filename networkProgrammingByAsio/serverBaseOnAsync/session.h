#pragma once
#include <iostream>
#include "boost/asio.hpp"
namespace asio = boost::asio;
namespace ip = asio::ip;
using tcp = ip::tcp;

const std::size_t max_length = 1024;

class Session{
public:
    Session(boost::asio::io_context& io):_socket(io){}

    tcp::socket& Socket(){
        return _socket;
    }

    void start(); //启动服务器，执行读写处理逻辑

private:
    void handle_read(const boost::system::error_code& ec,std::size_t bytes_transfered); //读的回调函数
    void handle_write(const boost::system::error_code& ec); //写的回调函数

    tcp::socket _socket;
    char _data[max_length];
};

class Server{
public:
    Server(asio::io_context& io,unsigned short port_num);

private:
    void start_accept();
    void handle_accept(Session* new_session,const boost::system::error_code& ec);//接收连接的回调函数
    asio::io_context& _ioc;
    tcp::acceptor _acceptor;
};