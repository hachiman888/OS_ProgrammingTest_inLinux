//v3旨在增加发送队列，防止数据在发送时乱序
//同时实现全双工通信（即随时双向通讯）

//v5旨在添加网络字节序和本地字节序的处理
//以及控制消息队列的大小，防止单一程序挤压其他程序的运行空间

//v6旨在添加json序列化处理
//v8旨在解耦网络通信层和逻辑处理层，为后序的多线程模式铺路
#pragma once
#include "const.h"
#include "MsgNode.h"
#include <iostream>
#include "boost/asio.hpp"
#include <memory>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "server_v6.h"
#include <mutex>
#include <queue>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
namespace asio = boost::asio;
namespace ip = asio::ip;
using tcp = ip::tcp;

class Server;

class Session : public std::enable_shared_from_this<Session>{
public:
    Session(boost::asio::io_context& io,Server* server):_socket(io),_server(server),
    _b_head_parse(false),_b_close(false),
    _recv_Head_Node(std::make_shared<Msg_Node>(HEAD_TOTAL_LEN)) //v4修改此处构造函数，避免空指针
    {
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();//生成随机值
        _uuid = boost::uuids::to_string(a_uuid);
    }

    tcp::socket& getSocket(){
        return _socket;
    }

    std::string& getUuid(){
        return _uuid;
    } 

    void socketClose(){
        _socket.close();
        _b_close = true;
    }

    void send(char* msg,short max_len,short msg_id); //封装的发送接口
    void send(std::string msg,short msg_id);

    void start(); //启动服务器，执行读写处理逻辑

    ~Session(){
        std::cout << "Session destruction, delete this " << this << std::endl;
    }

private:
    void handle_Read(const boost::system::error_code& ec,
        std::size_t bytes_transfered,std::shared_ptr<Session> _self_shared); //读的回调函数
    void handle_Write(const boost::system::error_code& ec,std::shared_ptr<Session> _self_shared); //写的回调函数

    tcp::socket _socket;
    char _data[MAX_LENGTH];
    Server* _server;                              //方便使用server内的map来管理和删除会话
    std::string _uuid;                            //该会话唯一的id
    std::queue<std::shared_ptr<Send_Node>> _send_queue; //发送队列
    std::mutex _send_lock;                        //发送锁
    std::shared_ptr<Recv_Node> _recv_Msg_Node;     //存储收到的消息节点
    bool _b_head_parse;                           //用于表示是否处理完头部信息
    bool _b_close;                                //用于表示socket是否关闭
    std::shared_ptr<Msg_Node> _recv_Head_Node;    //存储的消息节点的头部信息
}; 