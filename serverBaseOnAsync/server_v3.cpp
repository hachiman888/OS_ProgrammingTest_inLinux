#include "server_v3.h"

Server::Server(asio::io_context& io,unsigned short port_num):_ioc(io),
_acceptor(_ioc,tcp::endpoint(tcp::v4(),port_num))
{
    start_Accept();
}

void Server::start_Accept(){
     std::shared_ptr<Session> new_session = std::make_shared<Session>(_ioc,this);
    _acceptor.async_accept(new_session->getSocket(),
    std::bind(&Server::handle_Accept,this,new_session,std::placeholders::_1));
    //此处bind返回一个函数对象给async_accept，new_session通过值传递，引用计数+1，
    //asio将bind返回的回调函数插入待触发函数的队列中
    //这意味着，只要回调函数未被调用，new_session就一定存活
    //回调函数触发后且执行完毕后，引用计数才会-1
}

void Server::handle_Accept(std::shared_ptr<Session> new_session,const boost::system::error_code& ec){
    if(ec){
        std::cerr << "accept error!" << std::endl;
    }
    else{
        new_session->start();
        _sessions.insert(std::make_pair(new_session->getUuid(),new_session)); //接收连接请求，并将会话插入至map管理
        //此处shared_ptr引用计数+1，保证new_session存活，不被shared_ptr自动释放；
    }
    start_Accept();
}