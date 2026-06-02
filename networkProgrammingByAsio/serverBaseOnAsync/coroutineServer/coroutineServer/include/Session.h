#pragma once
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <queue>
#include "../include/Server.h"
#include "../include/const.h"
#include "../include/MsgNode.h"
#include "../include/LogicSystem.h"

using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::awaitable;
using boost::asio::strand;
using boost::asio::use_awaitable;
namespace this_coro = boost::asio::this_coro;

class Server;
class LogicSystem;
class MsgNode;
class SendNode;
class RecvNode;

class Session : public std::enable_shared_from_this<Session>  //忘了写public，导致私有继承，shared_from_this无法拿到指针
{
public:
    Session(boost::asio::io_context& ioc,Server* server);
    boost::asio::ip::tcp::socket& getSocket();
    std::string getUuid();
    void start();
    void close();
    void send(const char* msg,short msg_id,short msg_len);
    void send(std::string msg,short msg_id);
    ~Session();
private:
    void HandleWrite(const boost::system::error_code& ec,std::shared_ptr<Session> selfShared);
    Server* _server;
    std::string _uuid;
    boost::asio::io_context& _ioc;
    boost::asio::ip::tcp::socket _socket;
    bool _b_close;
    std::mutex _send_lock;
    std::queue<std::shared_ptr<SendNode>> _send_queue; //使用发送队列保证发送信息的时序性，防止上层多次调用send函数，造成数据发送混乱
    std::shared_ptr<RecvNode> _recv_Msg_Node;
    std::shared_ptr<MsgNode> _recv_Head_Node;
};

class LogicNode{
    friend class LogicSystem;
public:
    LogicNode(std::shared_ptr<Session>,std::shared_ptr<RecvNode>);
private:
    std::shared_ptr<Session> _session;
    std::shared_ptr<RecvNode> _recv_node;
};