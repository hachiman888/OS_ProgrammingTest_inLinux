#include "../include/AsioIOServicePool.h"
#include <iostream>

AsioIOServicePool::AsioIOServicePool(std::size_t threadNum):
    _ioServices(threadNum),_works(threadNum)
{
    for(std::size_t i = 0;i < threadNum;i++){
        _works[i] = std::make_unique<Work>(_ioServices[i]);
    }
    //遍历多个ioc，创建多个线程
    for(std::size_t i = 0;i < threadNum;i++){
        _threads.emplace_back([this,i](){ //不小心在这写了&i，造成了悬垂引用
            _ioServices[i].run();
        });
    }
}

void AsioIOServicePool::Stop(){
    for(auto& work : _works)
        work.reset();
}

boost::asio::io_context& AsioIOServicePool::GetIOService(){
    auto& service = _ioServices[_nextIOService++];
    if(_nextIOService >= _ioServices.size())
        _nextIOService = 0;
    return service;
}

AsioIOServicePool& AsioIOServicePool::GetInstance(){
    static AsioIOServicePool instance(1); //cpp11后，这样的单例写法线程安全,且更为简单
    return instance;
}

AsioIOServicePool::~AsioIOServicePool(){
    std::cout << "AsioIOServicePool destructed..." << std::endl;
}