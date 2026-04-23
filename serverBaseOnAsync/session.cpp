#include "session.h"
#include <iostream>

void Session::start(){
    memset(_data,0,max_length);
    this->_socket.async_read_some(asio::buffer(_data,max_length),
    std::bind(&Session::handle_read,this,std::placeholders::_1,std::placeholders::_2));  
    //socket调用async_read_some，其函数底层会将该socket添加到epoll的监听表中，此处监听读事件
    //当有读事件就绪（socket的读缓存区非空），系统自动触发回调函数
    //asio框架会自动把数据读进buffer中，数据会被存储进 _data中


    //tcp中，若对端关闭，服务器端会触发读事件，进而触发读的回调函数
}

void Session::handle_read(const boost::system::error_code& ec,std::size_t bytes_transfered){
    if(ec){
        std::cerr << "read error!" << std::endl;
        delete this;
        return ;
    }
    else{
        std::cout << "server received data is" << _data << std::endl;
        
        /*  一般的设计是持续监听服务器的读事件，保证全双工
        memset(_data,0,max_length);
        this->_socket.async_read_some(asio::buffer(_data,max_length),
            std::bind(handle_read,this,std::placeholders::_1,std::placeholders::_2));
        */

        asio::async_write(_socket,asio::buffer(_data,bytes_transfered),
        std::bind(&Session::handle_write,this,std::placeholders::_1));
        //发送缓存区非满，代表可写，系统调用写的回调函数
        //除非一次写完，或者触发错误，不然不会回调

        //若服务器读到数据后，将要去发送数据给对端，可此时对端关闭，在handle_write中delete了一次，
        //对端关闭同时会触发服务器的读事件就绪，于是在读事件中又delete一次，二次释放同一资源，有极大风险
        //问题在于延迟session的生命周期
        
        //此处不会触发是因为async系列函数都是一次性的
        //即当某个socket注册了异步操作之后，系统内核会将该socket加入监听事件表中，当事件触发，回调函数调用之后
        //该socket就会被移出监听事件表中
    }
}

void Session::handle_write(const boost::system::error_code& ec){
    if(ec){
        std::cerr << "write error!" << std::endl;
        delete this;
        return ;
    }
    else{
        memset(_data,0,max_length);
        this->_socket.async_read_some(asio::buffer(_data,max_length),
            std::bind(&Session::handle_read,this,std::placeholders::_1,std::placeholders::_2));
    }
}

Server::Server(asio::io_context& io,unsigned short port_num):_ioc(io),
_acceptor(_ioc,tcp::endpoint(tcp::v4(),port_num))
{
    start_accept();
}

void Server::start_accept(){
     Session* new_session = new Session(_ioc);
    _acceptor.async_accept(new_session->Socket(),
    std::bind(&Server::handle_accept,this,new_session,std::placeholders::_1));
}

void Server::handle_accept(Session* new_session,const boost::system::error_code& ec){
    if(ec){
        std::cerr << "accept error!" << std::endl;
        delete new_session;
        return ;
    }
    else{
        new_session->start();
    }
    start_accept();
}