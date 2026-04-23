#include "session_version2.h"
#include <iostream>

void Session::start(){ 
    //要在此处使得shared_ptr指向对象本身，绝不能使用std::make_shared<Session>(this)
    //这样会创建另外一个智能指针来指向同一块内存地址，导致多次释放同一块内存
    //shared_ptr仅在被复制时才会引用计数+1

    //为了解决这个问题，需要使得Session继承一个模板类，
    //即std::enable_shared_from_this
    memset(_data,0,max_length);
    this->_socket.async_read_some(asio::buffer(_data,max_length),
    std::bind(&Session::handle_read,this,std::placeholders::_1,std::placeholders::_2,shared_from_this()));
    //这样才能安全地将对象本身传入，并使得引用计数+1，保证对象本身生命周期  
}

//在同一个事件中，不该同时挂起async_write和async_read，这会导致引用计数永远不会减到0
void Session::handle_read(const boost::system::error_code& ec,std::size_t bytes_transfered,   
    std::shared_ptr<Session> _self_shared)
    {
    if(ec){
        std::cerr << "read error!" << std::endl;
        _server->clearSession(_uuid);
    }
    else{
        std::cout << "server received data is " << _data << std::endl;

        asio::async_write(_socket,asio::buffer("hello client!"),
        std::bind(&Session::handle_write,this,std::placeholders::_1,_self_shared));
        //在bind内值传递_self_shared，使得_self_shared的生命周期和回调函数对象生命周期一致
        //即若函数对象还在asio维护的待触发函数队列中，尚未被调用，那么Session对象一定存在
    }
}

void Session::handle_write(const boost::system::error_code& ec,std::shared_ptr<Session> _self_shared){
    if(ec){
        std::cerr << "write error!" << std::endl;
        _server->clearSession(_uuid);
    }
    else{
        memset(_data,0,max_length);
        this->_socket.async_read_some(asio::buffer(_data,max_length),
            std::bind(&Session::handle_read,this,std::placeholders::_1,std::placeholders::_2,_self_shared));
        //在bind内值传递_self_shared，使得_self_shared的生命周期和回调函数对象生命周期一致
        //即若函数对象还在asio维护的待触发函数队列中，尚未被调用，那么Session对象一定存在
    }
}

Server::Server(asio::io_context& io,unsigned short port_num):_ioc(io),
_acceptor(_ioc,tcp::endpoint(tcp::v4(),port_num))
{
    start_accept();
}

void Server::start_accept(){
     std::shared_ptr<Session> new_session = std::make_shared<Session>(_ioc,this);
    _acceptor.async_accept(new_session->GetSocket(),
    std::bind(&Server::handle_accept,this,new_session,std::placeholders::_1));
    //此处bind返回一个函数对象给async_accept，new_session通过值传递，引用计数+1，
    //asio将bind返回的回调函数插入待触发函数的队列中
    //这意味着，只要回调函数未被调用，new_session就一定存活
    //回调函数触发后且执行完毕后，引用计数才会-1
}

void Server::handle_accept(std::shared_ptr<Session> new_session,const boost::system::error_code& ec){
    if(ec){
        std::cerr << "accept error!" << std::endl;
    }
    else{
        new_session->start();
        _sessions.insert(std::make_pair(new_session->GetUuid(),new_session)); //接收连接请求，并将会话插入至map管理
        //此处shared_ptr引用计数+1，保证new_session存活，不被shared_ptr自动释放；
    }
    start_accept();
}