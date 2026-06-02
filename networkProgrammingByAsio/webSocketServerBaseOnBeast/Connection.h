#pragma once
#include <iostream>
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <memory>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <queue>
#include <mutex>

namespace net = boost::asio;
namespace beast = boost::beast;
using namespace boost::beast;
using namespace boost::beast::websocket;

//class ConnectionMgr;  //前置声明，防止头文件互相引用
class Connection : public std::enable_shared_from_this<Connection>
{
public:
    Connection(net::io_context& ioc);
    std::string GetUid();
    net::ip::tcp::socket& GetSocket(); //用于返回websocket管理的底层tcp的socket
    void AsyncAccept();     //当连接在tcp层面建立好之后，需要websocket层进行一次协议升级,该函数用于协议升级
    void Start();           //用于启动监听收发事件
    void AsyncSend(std::string msg);
    void SendCallBack(std::string msg);
private:
    std::unique_ptr<stream<tcp_stream>> _ws_ptr; //用unique_ptr管理websocket（面向消息流，基于tcp）
    std::string _uuid;  //使用唯一id，用于管理这些connection
    net::io_context& _ioc;
    beast::flat_buffer _recv_buffer; //存储数据
    std::queue<std::string> _send_queue; //发送队列
    std::mutex _send_mtx;  //保证发送时序性   
};