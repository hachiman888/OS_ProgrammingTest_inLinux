#include "session.h"

void Session::connect(const tcp::endpoint& ep){
    _socket->connect(ep);
    return ;
}

void Session::WritetoSocket(const std::string& buf){
    _queue.emplace(std::make_shared<Msg_Node>(buf.c_str(),buf.size()));
    if(_send_pending)
        return;//防止该函数处于未决状态时被多次调用，提前返回
    
    //异步发送数据，服务器端不应该使用一次调用保证发完的api，所以使用async_write_some或async_send;
    this->_socket->async_write_some(asio::buffer(buf),
    std::bind(WriteCallback,this,
        std::placeholders::_1,std::placeholders::_2));
        _send_pending = true; //设置发送状态为未决，之后将状态交给回调函数管理
}

void Session::WriteCallback(const boost::system::error_code& ec,
    std::size_t bytes_transferred){
    if (ec.value() != 0) { //函数参数由asio引擎自动填入，若ec不为0，则报错退出
        std::cout << "Error , code is " << ec.value() << " . Message is " << ec.message();
        return;
    }

    auto& send_data = _queue.front();
    send_data->_cur_len += bytes_transferred;//更新发送一部分数据后的数据偏移量

    //若数据未发完，则继续发送
    if(send_data->_cur_len < send_data->_total_len){
        this->_socket->async_write_some(asio::buffer(send_data->_msg + send_data->_cur_len,
            send_data->_total_len - send_data->_cur_len),
            std::bind(WriteCallback,this,std::placeholders::_1,std::placeholders::_2));
            return ;//交接接力棒，提前返回，防止执行if之后的代码
    }

    //数据发送完毕，出队，根据队列情况恢复非未决状态
    _queue.pop();
    if(_queue.empty()){
        _send_pending = false;
    }

    //若队列不为空，则接着处理下一个消息节点
    if(!_queue.empty()){
        auto& send_data = _queue.front();
        this->_socket->async_write_some(asio::buffer(send_data->_msg + send_data->_cur_len,
            send_data->_total_len - send_data->_cur_len),
            std::bind(WriteCallback,this,std::placeholders::_1,std::placeholders::_2));
    }
}

//不考虑粘包问题
void Session::ReadFromSocket(){
    if(_recv_pending){
        return ;
    }

    _recv_node = std::make_shared<Msg_Node>(RECVSIZE); //开辟读数据缓冲区
    this->_socket->async_read_some(asio::buffer(_recv_node->_msg,_recv_node->_total_len),
        std::bind(ReadCallback,this,
            std::placeholders::_1,std::placeholders::_2));
    _recv_pending = true;
}

void Session::ReadCallback(const boost::system::error_code& ec,
    std::size_t bytes_treasfered){
    if(ec.value() != 0){
        std::cout << "Error , code is " << ec.value() << " . Message is " << ec.message();
        return;
    }

    _recv_node->_cur_len += bytes_treasfered;
    //没读完就继续读
    if(_recv_node->_cur_len < _recv_node->_total_len){
        this->_socket->async_read_some(asio::buffer(_recv_node->_msg + _recv_node->_cur_len,
            _recv_node->_total_len - _recv_node->_cur_len),
            std::bind(ReadCallback,this,std::placeholders::_1,std::placeholders::_2));
        return ;
    }

    //此处省去将数据放入队列中，丢给线程处理的逻辑
    //数据读完则设置标志位
    _recv_pending = false;
    _recv_node = nullptr;
}