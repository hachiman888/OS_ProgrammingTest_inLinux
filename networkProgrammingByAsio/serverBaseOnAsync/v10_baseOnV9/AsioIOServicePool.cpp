#include "AsioIOServicePool.h"

AsioIOServicePool::AsioIOServicePool(std::size_t size):_ioServices(size),_works(size),
    _nextIOService(0)
{
    for(std::size_t i = 0;i < size;i++){
        _works[i] = std::make_unique<Work>(_ioServices[i]); //每个work对象绑定一个io_context对象
    }

    //遍历多个io_context，启动线程
    _threads.reserve(size);
    for(std::size_t i = 0;i < _ioServices.size();i++){
        _threads.emplace_back([this,i](){
            _ioServices[i].run(); //若work析构，则run在没任务执行时就会返回，io_context退出，以致线程退出
        });
    }
}

AsioIOServicePool::~AsioIOServicePool(){
    std::cout << "AsioIOServicePool destructed..." << std::endl; 
}

boost::asio::io_context& AsioIOServicePool::GetIOService(){
    auto& service = _ioServices[_nextIOService++]; //取出一个，索引自动+1
    if(_nextIOService == _ioServices.size()){
        _nextIOService = 0;
    }

    return service;
}

void AsioIOServicePool::Stop(){
    for(auto& work : _works){
        work.reset();   
        //释放掉work对象绑定的io_context，使得其没任务时不再阻塞，保证其能够退出，所有事件被解除
    }

    /*
    for(auto& t : _threads){
        t.join();
    }
        这里用的是jthread，不需要手动析构
    */
}