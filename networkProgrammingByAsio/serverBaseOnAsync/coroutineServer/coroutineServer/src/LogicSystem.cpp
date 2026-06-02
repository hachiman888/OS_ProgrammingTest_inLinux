#include "../include/LogicSystem.h"

LogicSystem::LogicSystem() : _b_stop(false){ //成员变量忘了初始化
    RegisterCallBackFunc();
    _worker_thread = std::thread(&LogicSystem::DealMsg,this);
}

LogicSystem::~LogicSystem(){
    _b_stop = true;
    _consumer.notify_one();
    _worker_thread.join();
}

void LogicSystem::PostMsgToQueue(std::shared_ptr<LogicNode> msg){
    std::unique_lock<std::mutex> locker(_logic_mutex);
    _msg_queue.push(msg);
    
    if(_msg_queue.size() == 1){ //此时队列非空，唤醒消费者
        locker.unlock();
        _consumer.notify_one();
    }
}

void LogicSystem::DealMsg(){ 
    for(;;){
        std::unique_lock<std::mutex> unique_locker(_logic_mutex);
        while(_msg_queue.empty() && !_b_stop){ //如果停服，则直接走到下一个if
            _consumer.wait(unique_locker);
        }

        if(_b_stop){ //停服，取出队列中所有数据，处理并退出
            while(!_msg_queue.empty()){
                auto msg = _msg_queue.front();
                std::cout << "msg id is " << msg->_recv_node->_msg_id << std::endl;
                auto it = _fun_callback.find(msg->_recv_node->_msg_id);
                if(it == _fun_callback.end()){ //若没找到，则直接再次循环
                    _msg_queue.pop();
                    continue;
                }
                //找到了，则调用其对应的回调函数
                it->second(msg->_session,
                    msg->_recv_node->_msg_id,
                    std::string(msg->_recv_node->_data,msg->_recv_node->_total_len));
                _msg_queue.pop();
            }
            break; //清空队列，停服
        }

        //若没有停服，且队列中有数据
        auto msg = _msg_queue.front();
        std::cout << "msg id is " << msg->_recv_node->_msg_id << std::endl;

        auto it = _fun_callback.find(msg->_recv_node->_msg_id);
        if(it == _fun_callback.end()){ //若没找到，则直接再次循环
            _msg_queue.pop();
            continue;
        }
        //找到了，则调用其对应的回调函数
        it->second(msg->_session,
                    msg->_recv_node->_msg_id,
                    std::string(msg->_recv_node->_data,msg->_recv_node->_total_len));
        _msg_queue.pop();
    }
}

void LogicSystem::RegisterCallBackFunc(){
    _fun_callback[static_cast<short>(MSG_IDS::MSG_HELLO_WORLD)] = 
        std::bind(&LogicSystem::HelloWorldCallBack,this,
            std::placeholders::_1,std::placeholders::_2,std::placeholders::_3);
}

void LogicSystem::HelloWorldCallBack
(std::shared_ptr<Session> session,const short& msg_id,const std::string& msg_data)
{
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);       //将字符串反序列化为类
    std::cout << "recevie msg id  is " << root["id"].asInt() << " msg data is "
        << root["data"].asString() << std::endl;
    root["data"] = "server has received msg, msg data is " + root["data"].asString();
    std::string return_str = root.toStyledString(); //处理后，序列化为字符串，回发给对端
    session->send(return_str, root["id"].asInt());
}

LogicSystem& LogicSystem::GetInstance(){
    static LogicSystem Instance;
    return Instance;
}
