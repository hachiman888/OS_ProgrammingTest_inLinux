#include <iostream>
#include <boost/asio.hpp>
#include <queue>
#include <set>
#include <memory>
#include <thread>

namespace asio = boost::asio;
namespace ip = asio::ip;
using tcp = ip::tcp;

const std::size_t RECVSIZE = 1024;

class Msg_Node{
public:
    Msg_Node(const char* msg,int total_len):_total_len(total_len){ //构造写节点
        _msg = new char[_total_len];
        memcpy(_msg,msg,_total_len);
    }

    Msg_Node(int total_len):_total_len(total_len){ //构造写节点
        _msg = new char[_total_len];
    }

    ~Msg_Node(){
        delete []_msg;
    }

public:
    char* _msg;//字符串首地址
    int _total_len; //字符串总长
    int _cur_len; //当前偏移量
};


class Session{
public:
    Session(std::shared_ptr<tcp::socket> sckt):_socket(sckt),
    _send_pending(false),_recv_pending(false){}
    void connect(const boost::asio::ip::tcp::endpoint& ep);

    void WritetoSocket(const std::string& buf);
    void WriteCallback(const boost::system::error_code& ec,
        std::size_t  bytes_transfered);

    void ReadFromSocket();
    void ReadCallback(const boost::system::error_code& ec,
        std::size_t bytes_treasfered);

private:
    std::shared_ptr<tcp::socket> _socket;
    std::queue<std::shared_ptr<Msg_Node>>_queue; //管理要发送的节点的先后顺序，保证消息不乱序
    bool _send_pending; //判断发送状态是否处于未决

    //boost::shared_ptr<Msg_Node> _recv_node;
    std::shared_ptr<Msg_Node> _recv_node; //缓存接收的数据
    bool _recv_pending;//判断接收状态是否处于未决
};