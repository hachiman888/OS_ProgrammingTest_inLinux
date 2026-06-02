#include "session_v9.h"
#include <iostream>

void Session::start(){ 
    memset(_data,0,MAX_LENGTH);
    this->_socket.async_read_some(asio::buffer(_data,MAX_LENGTH),
        boost::asio::bind_executor(_strand,
            std::bind(&Session::handle_Read,this,
                std::placeholders::_1,std::placeholders::_2,shared_from_this()))); 
                //将回调函数投递到strand维护的队列中执行 
}

void Session::handle_Read(const boost::system::error_code& ec,std::size_t bytes_transfered,   
    std::shared_ptr<Session> _self_shared)//v4修改headle_Read的实现，增加粘包处理
    {
    if(ec){
        std::cerr << "read error!" << std::endl;
        _server->clearSession(_uuid);
    }
    else{
        int copy_len = 0;//缓冲区_data的偏移管理变量
        while(bytes_transfered > 0) //有数据则执行，无数据则退出
        {
            if(!_b_head_parse) //如果头节点尚未被处理，则执行此处逻辑
            {
                //如果消息节点的头部信息所存储的长度 + 当前已读取的bytes 仍然小于 规定消息头长度，则执行
                //此处无需记录_data的偏移位置copy_len,因为_data已经全塞进去了，但还是不够4字节
                if(bytes_transfered + _recv_Head_Node->_cur_len < HEAD_TOTAL_LEN) 
                {   
                    //将已读到tcp缓冲区的数据，复制到消息节点的头部信息中
                    memcpy(_recv_Head_Node->_data + _recv_Head_Node->_cur_len,_data + copy_len,bytes_transfered);
                    _recv_Head_Node->_cur_len += bytes_transfered;//记录当前存储到消息节点头部的长度
                    ::memset(_data,0,MAX_LENGTH);//清空缓冲区，用于再次读
                    _socket.async_read_some(asio::buffer(_data,MAX_LENGTH),
                        boost::asio::bind_executor(_strand,
                            std::bind(&Session::handle_Read,this,
                                std::placeholders::_1,std::placeholders::_2,shared_from_this()
                            )
                        )
                    );
                    //将回调函数通过strand投递
                    return ;
                }

                //收到的数据长度比规定头部长度长，则执行以下逻辑
                //头部剩余的还未复制的长度
                int head_remain = HEAD_TOTAL_LEN - _recv_Head_Node->_cur_len;
                ::memcpy(_recv_Head_Node->_data + _recv_Head_Node->_cur_len,_data + copy_len,head_remain);
                copy_len += head_remain;    //更新已处理的_data长度和剩余未处理的长度
                bytes_transfered -= head_remain;

                //先处理消息id
                short msg_id = 0;
                memcpy(&msg_id,_recv_Head_Node->_data,HEAD_ID_LEN);
                msg_id = asio::detail::socket_ops::network_to_host_short(msg_id); //网络字节序转换为本地字节序
                std::cout << "msg id is " << msg_id << std::endl;

                if(msg_id > MAX_LENGTH) { //防御性编程
                    std::cout << "invaild msg_id is " << msg_id << std::endl;
                    _server->clearSession(_uuid);
                    return ;
                }

                //开始获取并解析头部记录的字节长度
                short msg_len = 0;
                memcpy(&msg_len,_recv_Head_Node->_data + HEAD_ID_LEN,HEAD_DATA_LEN);
                msg_len = asio::detail::socket_ops::network_to_host_short(msg_len); //转换为本地字节序
                std::cout << "the len of data is " << msg_len << std::endl;
                //若消息头部所记录的消息长度非法
                if(msg_len > MAX_LENGTH) 
                {
                    std::cout << "invaild data length is " << msg_len << std::endl;
                    _server->clearSession(_uuid);
                    return ;
                }
                _recv_Msg_Node = std::make_shared<Recv_Node>(msg_len,msg_id);//构造消息体

                //如果readsome读到的字节数小于头部规定的长度，则说明数据还未接收完，先存储进入消息节点中
                if(bytes_transfered < msg_len)
                {   
                    //此处_data + copy_len使得_data偏移到仍未被复制的位置
                    memcpy(_recv_Msg_Node->_data + _recv_Msg_Node->_cur_len,_data + copy_len,bytes_transfered);
                    _recv_Msg_Node->_cur_len += bytes_transfered;
                    ::memset(_data,0,MAX_LENGTH); //清空，接着读
                    _socket.async_read_some(asio::buffer(_data,MAX_LENGTH),
                        boost::asio::bind_executor(_strand,
                            std::bind(&Session::handle_Read,this,
                                std::placeholders::_1,std::placeholders::_2,shared_from_this()
                            )
                        )
                    );
                    //将回调函数通过strand投递
                    _b_head_parse = true;   //标记头部处理完成，使得下次触发回调时，进入处理消息体逻辑
                    return ;
                }

                //若上一次小于，下一次大于，则不会执行以下逻辑，会执行if(!_b_head_parse)外的逻辑
                //此处是处理并读取头节点内规定的消息长度后，对消息节点内容的第一次填充的逻辑
                //如果读到的字节数大于头部规定的长度，则有冗余
                memcpy(_recv_Msg_Node->_data + _recv_Msg_Node->_cur_len,_data + copy_len,msg_len);
                _recv_Msg_Node->_cur_len += msg_len;
                copy_len += msg_len; //更新copy_len，使得_data正确偏移
                bytes_transfered -= msg_len;
                _recv_Msg_Node->_data[_recv_Msg_Node->_total_len] = '\0';//主动添加\0
                LogicSystem::GetInstance()->PostMsgToQueue(std::make_shared<LogicNode>(shared_from_this(),_recv_Msg_Node));
                _b_head_parse = false; //本次消息节点处理完毕
                _recv_Head_Node->Clear();//复用节点
                if(bytes_transfered <= 0){
                    ::memset(_data,0,MAX_LENGTH);
                    _socket.async_read_some(asio::buffer(_data,MAX_LENGTH),
                        boost::asio::bind_executor(_strand,
                            std::bind(&Session::handle_Read,this,
                                std::placeholders::_1,std::placeholders::_2,shared_from_this()
                            )
                        )
                    );
                    //将回调函数通过strand投递
                    return ;
                }
                continue;
            }

            //此处为已经处理完头部，要处理上次未接收完的数据
            //若tcp缓冲区中读到的数据仍未达到 消息节点剩余的长度，则再读
            int remain_msg = _recv_Msg_Node->_total_len - _recv_Msg_Node->_cur_len;
            if(bytes_transfered < remain_msg)
            {
                memcpy(_recv_Msg_Node->_data + _recv_Msg_Node->_cur_len,_data + copy_len,bytes_transfered);
                _recv_Msg_Node->_cur_len += bytes_transfered;
                memset(_data,0,MAX_LENGTH);
                _socket.async_read_some(asio::buffer(_data,MAX_LENGTH),
                    boost::asio::bind_executor(_strand,
                        std::bind(&Session::handle_Read,this,
                            std::placeholders::_1,std::placeholders::_2,shared_from_this()
                        )
                    )
                );
                //将回调函数通过strand投递
                return ;
            }

            //若bytes_transfered >= 消息节点的剩余长度
            //则只从bytes_transfered读取目标剩余长度
            memcpy(_recv_Msg_Node->_data + _recv_Msg_Node->_cur_len,_data + copy_len,remain_msg);
            _recv_Msg_Node->_cur_len += remain_msg;
            bytes_transfered -= remain_msg;
            copy_len += remain_msg;
            _recv_Msg_Node->_data[_recv_Msg_Node->_total_len] = '\0';
            LogicSystem::GetInstance()->PostMsgToQueue(std::make_shared<LogicNode>(shared_from_this(),_recv_Msg_Node));
            _b_head_parse = false; //继续处理剩余数据（bytes_transfered中的残余数据）
            _recv_Head_Node->Clear(); //清空复用
            if(bytes_transfered <= 0){
                    ::memset(_data,0,MAX_LENGTH);
                    _socket.async_read_some(asio::buffer(_data,MAX_LENGTH),
                        boost::asio::bind_executor(_strand,
                            std::bind(&Session::handle_Read,this,
                                std::placeholders::_1,std::placeholders::_2,shared_from_this()
                            )
                        )
                    );//将回调函数通过strand投递
                    return ;
            }
            continue ;
        }
    }
}

void Session::handle_Write(const boost::system::error_code& ec,std::shared_ptr<Session> _self_shared){
    //添加异常处理
    try{
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
                    boost::asio::bind_executor(_strand,std::bind(&Session::handle_Write,this,
                        std::placeholders::_1,shared_from_this()))); //将回调函数通过strand投递
            }
        }
    }
    catch(std::exception& e){
        std::cerr << "exception occur! exception is " << e.what() << std::endl;
    }
}

void Session::send(char* msg,short max_len,short msg_id){
    std::lock_guard<std::mutex> locker(_send_lock); 
    std::size_t send_queue_size = _send_queue.size();
    if(send_queue_size > MAX_SEND_QUEUE){
        std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SEND_QUEUE << std::endl;
        return ;
    } 

    _send_queue.emplace(std::make_shared<Send_Node>(msg,max_len,msg_id)); //先将当前数据加入队列
    if(send_queue_size > 0){ //若队列中仍存在数据，说明队列中残留有未发完的数据
        return ;
    }

    auto& msg_node = _send_queue.front();
    asio::async_write(_socket,asio::buffer(msg_node->_data,msg_node->_total_len),
        boost::asio::bind_executor(_strand,std::bind(&Session::handle_Write,this,
            std::placeholders::_1,shared_from_this()))); //将回调函数通过strand投递
}

void Session::send(std::string msg,short msg_id){
    std::lock_guard<std::mutex> locker(_send_lock);
    std::size_t send_queue_size = _send_queue.size();
    if(send_queue_size > MAX_SEND_QUEUE){
        std::cout << "session: " << _uuid << " send que fulled, size is " << MAX_SEND_QUEUE << std::endl;
        return ;
    }

    _send_queue.emplace(std::make_shared<Send_Node>(msg.c_str(),msg.size(),msg_id));
    if(send_queue_size > 0){
        return ;
    }

    auto& msg_node = _send_queue.front();
    asio::async_write(_socket,asio::buffer(msg_node->_data,msg_node->_total_len),
        boost::asio::bind_executor(_strand,std::bind(&Session::handle_Write,this,
            std::placeholders::_1,shared_from_this())));
}

LogicNode::LogicNode(std::shared_ptr<Session> session,std::shared_ptr<Recv_Node> recv_node)
    : _session(session),_recv_node(recv_node){}