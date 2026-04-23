//v3旨在增加发送队列，防止数据在发送时乱序
//同时实现全双工通信（即随时双向通讯）

#pragma once
#include <iostream>
#include "boost/asio.hpp"
#include <memory>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "server_v3.h"
#include <mutex>
#include <queue>
namespace asio = boost::asio;
namespace ip = asio::ip;
using tcp = ip::tcp;

const std::size_t max_length = 1024;

class Server;

class Msg_Node{
    friend class Session;
public:
    Msg_Node(char* msg,std::size_t len):_cur_len(0){
        _data = new char[len];
        memcpy(_data,msg,len);
    }

    ~Msg_Node(){
        delete[] _data;
    }

private:
    char* _data;
    std::size_t _cur_len;
    std::size_t _total_len;
};

class Session : public std::enable_shared_from_this<Session>{
public:
    Session(boost::asio::io_context& io,Server* server):_socket(io),_server(server){
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();//生成随机值
        _uuid = boost::uuids::to_string(a_uuid);
    }

    tcp::socket& getSocket(){
        return _socket;
    }

    std::string& getUuid(){
        return _uuid;
    } 

    void send(char* msg,std::size_t max_length); //封装的发送接口

    void start(); //启动服务器，执行读写处理逻辑

    ~Session(){
        std::cout << "Session destruction, delete this" << this << std::endl;
    }

private:
    void handle_Read(const boost::system::error_code& ec,
        std::size_t bytes_transfered,std::shared_ptr<Session> _self_shared); //读的回调函数
    void handle_Write(const boost::system::error_code& ec,std::shared_ptr<Session> _self_shared); //写的回调函数

    tcp::socket _socket;
    char _data[max_length];
    Server* _server; //方便使用server内的map来管理和删除会话
    std::string _uuid; //该会话唯一的id
    std::queue<std::shared_ptr<Msg_Node>> _send_queue;
    std::mutex _send_lock;
};