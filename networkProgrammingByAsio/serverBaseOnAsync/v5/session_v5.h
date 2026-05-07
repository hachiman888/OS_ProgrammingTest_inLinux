//v3旨在增加发送队列，防止数据在发送时乱序
//同时实现全双工通信（即随时双向通讯）

//v5旨在添加网络字节序和本地字节序的处理
//以及控制消息队列的大小，防止单一程序挤压其他程序的运行空间
#pragma once
#include <iostream>
#include "boost/asio.hpp"
#include <memory>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include "server_v5.h"
#include <mutex>
#include <queue>
namespace asio = boost::asio;
namespace ip = asio::ip;
using tcp = ip::tcp;

#define max_length  2048
#define HEAD_LENGTH  2
#define MAX_SEND_QUEUE 1000

class Server;

class Msg_Node{
    friend class Session;  //v4版本，修改消息节点，使得消息节点维护消息长度和消息内容
    //旨在解决粘包问题
public:
    //修改原消息节点的构造函数
    //该构造函数主要用于发送数据时构造发送信息的节点
    Msg_Node(char* msg,short max_len):_total_len(max_len + HEAD_LENGTH),_cur_len(0){
        _data = new char[_total_len + 1]; //+1给\0留一个位置
        short max_len_host = asio::detail::socket_ops::host_to_network_short(max_len);//本地字节序转为网络字节序
        memcpy(_data,&max_len_host,HEAD_LENGTH); //先将长度信息放进消息节点中
        memcpy(_data + HEAD_LENGTH,msg,max_len);//之后_data偏移两位，从这个位置复制消息内容
        _data[_total_len] = '\0';
    }

    //主要根据消息的长度构造消息节点，该构造函数主要是接收对端数据时构造接收节点调用的构造函数。
    Msg_Node(unsigned short max_len):_total_len(max_len),_cur_len(0){
        _data = new char[_total_len + 1];
    }

    ~Msg_Node(){
        delete[] _data;
    }

    //新增一个Clear函数清除消息节点的数据，主要是避免多次构造节点造成开销。
    void Clear(){
        ::memset(_data,0,_total_len);
        _cur_len = 0;
    }

private:
    char* _data;
    std::size_t _cur_len;
    std::size_t _total_len;
};

class Session : public std::enable_shared_from_this<Session>{
public:
    Session(boost::asio::io_context& io,Server* server):_socket(io),_server(server),
    _b_head_parse(false),_recv_Head_Node(std::make_shared<Msg_Node>(max_length)) //v4修改此处构造函数，避免空指针
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

    void send(char* msg,short max_len); //封装的发送接口

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
    Server* _server;                              //方便使用server内的map来管理和删除会话
    std::string _uuid;                            //该会话唯一的id
    std::queue<std::shared_ptr<Msg_Node>> _send_queue; //发送队列
    std::mutex _send_lock;                        //发送锁
    std::shared_ptr<Msg_Node> _recv_Msg_Node;     //存储收到的消息节点
    bool _b_head_parse;                           //用于表示是否处理完头部信息
    std::shared_ptr<Msg_Node> _recv_Head_Node;    //存储的消息节点的头部信息
}; 