#include "LogicSystem.h"

LogicSystem::LogicSystem():_b_stop(false){
    RegisterCallBacks();
    _worker_thread = std::thread(&LogicSystem::DealMsg,this);
}

void LogicSystem::DealMsg(){  //此处锁粒度太大，需要改进，以及多线程同时输出问题待解决
    for(;;){
        std::unique_lock<std::mutex> locker(_mutex);
        //判断队列为空,则用条件变量阻塞等待，并释放锁；若bstop为true，且队列非空，直接跳出循环
        while(_msg_queue.empty() && !_b_stop){
            _consume.wait(locker);
        }

        //如果执行到此处时，停服，则工作线程取出队列中所有数据，及时处理
        if(_b_stop){
            while(!_msg_queue.empty()){
                auto msg_node = _msg_queue.front();
                std::cout << "msg id is " << msg_node->_recv_node->_msg_id << std::endl;

                auto call_back_iter = _fun_callback.find(msg_node->_recv_node->_msg_id);
                if(call_back_iter == _fun_callback.end()){ //若没找到msgid对应的回调函数，则跳过
                    _msg_queue.pop();
                    continue;
                }
                //若找到了，则执行对应的回调函数
                call_back_iter->second(msg_node->_session,
                    msg_node->_recv_node->_msg_id,
                    std::string(msg_node->_recv_node->_data,msg_node->_recv_node->_cur_len));
                _msg_queue.pop();
            }
            break; //停服，中断工作线程
        }

        //若没有停服，则取出队列中数据
        auto msg_node = _msg_queue.front();
        std::cout << "msg id is " << msg_node->_recv_node->_msg_id << std::endl;

        auto call_back_iter = _fun_callback.find(msg_node->_recv_node->_msg_id);
        if(call_back_iter == _fun_callback.end()){ //若没找到msgid对应的回调函数，则跳过
            _msg_queue.pop();
            continue;
        }
        //若找到了，则执行对应的回调函数
        call_back_iter->second(msg_node->_session,
        msg_node->_recv_node->_msg_id,
            std::string(msg_node->_recv_node->_data,msg_node->_recv_node->_cur_len));
        _msg_queue.pop();
    }
}

void LogicSystem::RegisterCallBacks(){
    _fun_callback[static_cast<short>(MSG_IDS::MSG_HELLO_WORLD)] = std::bind(&LogicSystem::HelloWorldCallBack,
        this,std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);
}

void LogicSystem::HelloWorldCallBack(std::shared_ptr<Session> session,
    const short& msg_id,const std::string& msg_data)
    {
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    std::cout << "recevie msg id  is " << root["id"].asInt() << " msg data is "
        << root["data"].asString() << std::endl;
    root["data"] = "server has received msg, msg data is " + root["data"].asString();
    std::string return_str = root.toStyledString();
    session->send(return_str, root["id"].asInt());
}

void LogicSystem::PostMsgToQueue(std::shared_ptr<LogicNode> node){
    std::unique_lock<std::mutex> unique_lk(_mutex); //操作逻辑队列前必须加锁
    _msg_queue.push(node);

    if(_msg_queue.size() == 1)
        _consume.notify_one();
}

LogicSystem::~LogicSystem(){
    _b_stop = true;
    _consume.notify_one();
    _worker_thread.join();
    std::cout << "LogicSystem was destructed..." << std::endl;
}