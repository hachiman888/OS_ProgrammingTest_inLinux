#include "session_v3.h"
#include <iostream>

//问题：未处理粘包问题

void Session::start(){ 
    memset(_data,0,max_length);
    this->_socket.async_read_some(asio::buffer(_data,max_length),
    std::bind(&Session::handle_Read,this,std::placeholders::_1,std::placeholders::_2,shared_from_this())); 
}

void Session::handle_Read(const boost::system::error_code& ec,std::size_t bytes_transfered,   
    std::shared_ptr<Session> _self_shared)
    {
    if(ec){
        std::cerr << "read error!" << std::endl;
        _server->clearSession(_uuid);
    }
    else{
        std::cout << "server received data is " << _data << std::endl;
        this->send(_data,bytes_transfered);
        //全双工设计中，服务器必须一直监听读事件
        memset(_data,0,1024);
        this->_socket.async_read_some(asio::buffer(_data,max_length),
        std::bind(&Session::handle_Read,this,std::placeholders::_1,std::placeholders::_2,shared_from_this()));
    }
}

void Session::handle_Write(const boost::system::error_code& ec,std::shared_ptr<Session> _self_shared){
    if(ec){
        std::cerr << "write error!" << std::endl;
        _server->clearSession(_uuid);
    }
    else{
        std::lock_guard<std::mutex> locker(_send_lock);
        _send_queue.pop();//此处先pop是因为async_write发送的是队首的数据，只有发送成功了，才会调用回调函数，所以先出队
        if(!_send_queue.empty()){
            auto& msg = _send_queue.front();
            asio::async_write(_socket,asio::buffer(msg->_data,msg->_total_len),
            std::bind(&Session::handle_Write,this,std::placeholders::_1,shared_from_this()));
        }
    }
}

void Session::send(char* msg,std::size_t max_length){
    bool pending = false;//先创建一个局部变量pending，pending为true时，队列内存在未发完的数据，false，队列为空
    std::lock_guard<std::mutex> locker(_send_lock); 
    if(_send_queue.size() > 0){ //若队列中仍存在数据，说明队列中残留有未发完的数据
        pending = true;
    }

    _send_queue.emplace(std::make_shared<Msg_Node>(msg,max_length)); //先将当前数据加入队列
    if(pending){ //再判断是否未决
        return;
    }

    asio::async_write(_socket,asio::buffer(msg,max_length),
    std::bind(&Session::handle_Write,this,std::placeholders::_1,shared_from_this()));
}