#include "../include/Session.h"
//为什么处理发送事件时，不适合用协程？
//协程转让控制权时，仍保留有栈信息。
//假设对端的读取速度太慢，导致缓冲区在多协程共同写的情况下经常满
//那么协程保留的数据会逐渐累加，最后导致服务器内存被耗尽，所以发送数据时，不适合用协程处理

Session::Session(boost::asio::io_context& ioc,Server* server):
    _ioc(ioc),_server(server),_socket(ioc),_b_close(false)
{
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        _uuid = boost::uuids::to_string(a_uuid);
        _recv_Head_Node = std::make_shared<MsgNode>(HEAD_TOTAL_LEN);
}


boost::asio::ip::tcp::socket& Session::getSocket(){
    return _socket;
}

std::string Session::getUuid(){
    return _uuid;
}

void Session::start(){
    //此处改为协程启动,每个协程独立处理一个会话的读事件
    auto shared_self = shared_from_this(); //引用计数+1，防止提前释放
    //开启协程接收
    co_spawn(_ioc,[shared_self,this]()->awaitable<void>{
        try{
            for(;!_b_close;){ //判断条件是否为true，若为true，则退出循环
                _recv_Head_Node->Clear(); //保证头部为空
                //启动一个协程函数
                std::size_t n = co_await boost::asio::async_read(_socket,boost::asio::buffer  
                    (_recv_Head_Node->_data,HEAD_TOTAL_LEN),
                    use_awaitable); //允许协程读不到时主动挂起，转移控制权
                
                if(n == 0) {
                    std::cout << "receive peer closed..." << std::endl;
                    close();
                    _server->clearSession(_uuid);
                    co_return; //协程返回关键字
                }

                //处理头部节点数据
                short msg_id = 0;
                memcpy(&msg_id,_recv_Head_Node->_data,HEAD_ID_LEN);
                //字节序转换
                msg_id = boost::asio::detail::socket_ops::network_to_host_short(msg_id);
                std::cout << "msg id is " << msg_id << std::endl;
                
                if(msg_id > MAX_LENGTH){
                    std::cerr << "invaild msg id is " << msg_id << std::endl;
                    close();
                    _server->clearSession(_uuid);
                    co_return; 
                }
                
                short msg_len = 0;
                memcpy(&msg_len,_recv_Head_Node->_data + HEAD_ID_LEN,HEAD_DATA_LEN);
                msg_len = boost::asio::detail::socket_ops::network_to_host_short(msg_len);
                std::cout << "msg len is " << msg_len << std::endl;

                if(msg_len > MAX_LENGTH){
                    std::cerr << "invaild msg len is " << msg_len << std::endl;
                    close();
                    _server->clearSession(_uuid);
                    co_return;
                }

                _recv_Msg_Node = std::make_shared<RecvNode>(msg_id,msg_len);
                //处理消息节点
                n = co_await boost::asio::async_read(_socket,
                        boost::asio::buffer(_recv_Msg_Node->_data,_recv_Msg_Node->_total_len),
                            use_awaitable);
                
                if(n == 0){
                    std::cerr << "receive peer closed..." << std::endl;
                    close();
                    _server->clearSession(_uuid);
                    co_return;
                }

                _recv_Msg_Node->_data[_recv_Msg_Node->_total_len] = '\0';//手动添加结束符，方便打印，好习惯
                std::cout << "received data is " << _recv_Msg_Node->_data << std::endl;

                //此处投放至逻辑线程处理
                LogicSystem::GetInstance().PostMsgToQueue
                (std::make_shared<LogicNode>(shared_from_this(),_recv_Msg_Node));
            }
        }catch(std::exception& e){
            std::cerr << "start error! exception is " 
                << e.what() << std::endl;
            close();
            _server->clearSession(_uuid);
        }
    },detached);
}

void Session::send(const char* msg,short msg_id,short msg_len){
    std::unique_lock<std::mutex> lock(_send_lock); //使用可以手动解锁的锁，降低锁的粒度
    std::size_t send_que_size = _send_queue.size();
    if(send_que_size > MAX_SENDQUE){
        std::cout << "session: " << _uuid 
            << "\tsend queue fulled,size is" << send_que_size << std::endl;
        return ;
    }

    _send_queue.push(std::make_shared<SendNode>(msg,msg_id,msg_len));
    if(send_que_size > 0) {
        return ; //队列里有数据时，只允许往队列里投放，不允许取
        //当会话多次调用send时，只有一次会真的触发写，其他次只是投放数据
    }

    auto msgNode = _send_queue.front();
    lock.unlock(); //降低锁粒度，增加程序并发性
    boost::asio::async_write(_socket,boost::asio::buffer(msgNode->_data,msgNode->_total_len),
        std::bind(&Session::HandleWrite,this,
            std::placeholders::_1,shared_from_this()));
}

void Session::send(std::string msg,short msg_id){
    send(msg.c_str(),msg_id,msg.size());
}

void Session::close(){
    _b_close = true;
    _socket.close();
}

void Session::HandleWrite(const boost::system::error_code& ec,std::shared_ptr<Session> selfShared){
    try{
        if(ec){
            std::cerr << "HandleWrite error! error code is: " << ec.value()
                << "error message is " << ec.what() << std::endl;
            close();
            _server->clearSession(_uuid); 
        }
        else{
            std::unique_lock<std::mutex> locker(_send_lock);
            _send_queue.pop();
            if(!_send_queue.empty()){ //若这次写完队列不为空，则接着写
                auto msgNode = _send_queue.front();
                locker.unlock();
                boost::asio::async_write(_socket,
                        boost::asio::buffer(msgNode->_data,msgNode->_total_len),
                            std::bind(&Session::HandleWrite,this,std::placeholders::_1,shared_from_this()));
            }
        }
    }catch(std::exception& e){
        std::cerr << "HandleWrite exception occured! exception is " 
            << e.what() << std::endl;
        close();
        _server->clearSession(_uuid); 
    }
}

Session::~Session(){
    try{
        std::cout << "Session destructed..." << std::endl;
        close();
    }catch(std::exception& e){
        std::cerr << "Session destructor error!error message is: " 
            << e.what() << std::endl; 
    }
}

LogicNode::LogicNode(std::shared_ptr<Session> session,
    std::shared_ptr<RecvNode> recv_node):_session(session),_recv_node(recv_node){}