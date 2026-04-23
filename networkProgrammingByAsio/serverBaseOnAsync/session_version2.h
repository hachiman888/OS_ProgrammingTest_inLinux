/*
    在version1中，在任何一个回调函数中碰到异常，就会直接delete this
    这会直接导致这次回调函数碰到异常，但asio管理的待触发的回调函数队列中仍有元素
    这会导致多次异常，并且多次delete this

    为此，将使用伪闭包的方式，使得即便异常，也保证（延长）session类的生命周期
    闭包：即一个函数，能够记住并访问它被创建时所在的作用域里的变量
    即便那个作用域已经执行完毕了

    我们可以通过智能指针的方式管理Session类，
    将acceptor接收的链接保存在Session类型的智能指针里。
*/

#pragma once
#include <iostream>
#include "boost/asio.hpp"
#include <memory>
#include <map>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
namespace asio = boost::asio;
namespace ip = asio::ip;
using tcp = ip::tcp;

const std::size_t max_length = 1024;

class Server;

class Session : public std::enable_shared_from_this<Session>{
public:
    Session(boost::asio::io_context& io,Server* server):_socket(io),_server(server){
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();//生成随机值
        _uuid = boost::uuids::to_string(a_uuid);
    }

    tcp::socket& GetSocket(){
        return _socket;
    }

    std::string& GetUuid(){
        return _uuid;
    } 

    void start(); //启动服务器，执行读写处理逻辑

    ~Session(){
        std::cout << "Session destruction, delete this" << this << std::endl;
    }

private:
    void handle_read(const boost::system::error_code& ec,
        std::size_t bytes_transfered,std::shared_ptr<Session> _self_shared); //读的回调函数
    void handle_write(const boost::system::error_code& ec,std::shared_ptr<Session> _self_shared); //写的回调函数

    tcp::socket _socket;
    char _data[max_length];
    Server* _server; //方便使用server内的map来管理和删除会话
    std::string _uuid; //该会话唯一的id
};

class Server{
public:
    Server(asio::io_context& io,unsigned short port_num);
    void clearSession(std::string uuid){
        _sessions.erase(uuid); //若uuid不存在，则无事发生
    }

private:
    void start_accept();
    void handle_accept(std::shared_ptr<Session> new_session,const boost::system::error_code& ec);//接收连接的回调函数
    asio::io_context& _ioc;
    tcp::acceptor _acceptor;
    std::map<std::string,std::shared_ptr<Session>> _sessions; //使用map来管理会话，有连接就插入，断开连接就删除
};