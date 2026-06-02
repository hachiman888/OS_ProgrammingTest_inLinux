#pragma once
#include "Singleton.h"
#include "session_v9.h"
#include "const.h"
#include <queue>
#include <map>
#include <thread>
#include <functional>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>

class Session;
class LogicNode;
//将shared_ptr传入类，或者函数，都是为了保证其对象的存活性，延长其生命周期
using FunCallBack = std::function<void(std::shared_ptr<Session>,const short& msg_id,const std::string& msg_data)>;

class LogicSystem : public Singleton<LogicSystem>{
    friend class Singleton<LogicSystem>; //声明友元，使得基类可以访问LogicSystem的构造函数，实例化基类成员变量
public:
    ~LogicSystem();
    void PostMsgToQueue(std::shared_ptr<LogicNode>);  //回调函数

private:
    LogicSystem();
    void RegisterCallBacks();                         //注册回调函数的函数
    void HelloWorldCallBack(std::shared_ptr<Session>,const short& msg_id,const std::string& msg_data);
    //专门处理msgid为1001的消息的回调函数
    void DealMsg(); //逻辑系统给工作线程调用的函数，用于处理消息
    
    std::queue<std::shared_ptr<LogicNode>> _msg_queue; //逻辑队列
    std::mutex _mutex;
    std::condition_variable _consume;                  //队列有可能为空，需要主动线程挂起，释放cpu资源
    std::thread _worker_thread;                        //逻辑系统中的工作线程，用于从逻辑队列中取出数据
    bool _b_stop;                                      //用于判断是否接收到顶层的中断信号，若收到，则工作线程会将逻辑队列中所有节点取出，及时处理后，才析构逻辑系统
    std::map<short,FunCallBack> _fun_callback;         //将msgid和回调函数绑定起来，

};