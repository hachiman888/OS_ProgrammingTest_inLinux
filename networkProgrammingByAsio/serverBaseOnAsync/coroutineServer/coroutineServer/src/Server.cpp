#include "../include/Server.h"
#include <iostream>


Server::Server(boost::asio::io_context& ioc,short port):
    _ioc(ioc),_port(port),
    _acceptor(_ioc,boost::asio::ip::tcp::endpoint(boost::asio::ip::make_address_v4("0.0.0.0"),_port))
{
    std::cout << "Server start success, listen on port : " << _port << std::endl;
    startAccept();
}

Server::~Server(){
    std::cout << "Server destruct listen on port : " << _port << std::endl;
}

void Server::startAccept(){
    auto& ioc = AsioIOServicePool::GetInstance().GetIOService();
    auto new_session = std::make_shared<Session>(ioc,this);
    _acceptor.async_accept(new_session->getSocket(),
        std::bind(&Server::HandleAccept,this,new_session,std::placeholders::_1));
}

void Server::HandleAccept(std::shared_ptr<Session> new_session,
    const boost::system::error_code& ec)
{
    if(ec){
        std::cerr << "session accepting error! error code is " << 
            ec.value() << "\terror message is " 
                << ec.what() << std::endl;
    }
    else{
        new_session->start();
        std::lock_guard<std::mutex> lk_guard(_mutex);
        _sessions.insert({new_session->getUuid(),new_session});
    }

    startAccept(); //处理完当前会话，再次注册监听事件
}

void Server::clearSession(std::string uuid){
    std::lock_guard<std::mutex> lk_guard(_mutex);
    std::cout << "session cleared..." << std::endl;
    _sessions.erase(uuid); //若uuid不存在，则无事发生
}