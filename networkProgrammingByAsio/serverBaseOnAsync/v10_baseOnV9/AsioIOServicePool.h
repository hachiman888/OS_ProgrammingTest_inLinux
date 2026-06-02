#pragma once
#include "Singleton.h"
#include <thread>
#include <boost/asio.hpp>
#include <vector>
#include <memory>

class AsioIOServicePool : public Singleton<AsioIOServicePool>
{
    friend class Singleton<AsioIOServicePool>;
public:
    using IOService = boost::asio::io_context;
    using Work = boost::asio::io_context::work; //用于防止io_context在无事件时提前退出,work类已被asio废弃
    //可替换成这样
    //using WORK = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    using WorkPtr = std::unique_ptr<Work>;  //WorkPtr是boost::asio::io_context::work类型的unique指针

    IOService& GetIOService();  // 使用 round-robin 的方式,获取一个io_context
    void Stop();    //停止IO服务池

    
    ~AsioIOServicePool();
    AsioIOServicePool(const AsioIOServicePool&) = delete;
    AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
private:
    AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());
    std::vector<IOService> _ioServices; //用于初始化多个io_context
    std::vector<std::jthread> _threads; // 线程池，用于管理所有的线程
    std::vector<WorkPtr> _works;  //和多个io_context配套的work对象
    std::size_t _nextIOService;   //表示需要返回的IOService的索引
};