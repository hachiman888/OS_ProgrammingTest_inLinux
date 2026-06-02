#pragma once
#include <map>
#include <queue>
#include <thread>
#include <functional>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include "../include/const.h"
#include "../include/Session.h"

class Session;
class LogicNode;
//将shared_ptr传入类，或者函数，都是为了保证其对象的存活性，延长其生命周期
using FunCallBack = std::function<void(std::shared_ptr<Session>,const short& msg_id,const std::string& msg_data)>;

class LogicSystem {
public:
    ~LogicSystem();
    void PostMsgToQueue(std::shared_ptr<LogicNode> msg);
    static LogicSystem& GetInstance();

    LogicSystem(const LogicSystem&) = delete;
    LogicSystem& operator=(const LogicSystem&) = delete;
private:
    LogicSystem();
    void DealMsg();     //逻辑系统给工作线程调用的函数，用于处理消息
    void RegisterCallBackFunc(); //注册回调函数，将回调函数和msgid绑定在一起
    void HelloWorldCallBack(std::shared_ptr<Session>,const short& msg_id,const std::string& msg_data);

    std::thread _worker_thread;         //逻辑系统中的工作线程，用于从逻辑队列中取出数据
    std::queue<std::shared_ptr<LogicNode>> _msg_queue;  //逻辑队列
    std::mutex _logic_mutex;
    std::condition_variable _consumer;  //队列有可能为空，需要主动让线程挂起，释放cpu资源
    bool _b_stop;       //用于判断是否接收到顶层的中断信号，若收到，则工作线程会将逻辑队列中所有节点取出，及时处理后，才析构逻辑系统
    std::map<short,FunCallBack> _fun_callback;  //将msgid和回调函数绑定起来，
};