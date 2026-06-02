#include "AsioThreadPool.h"

AsioThreadPool::AsioThreadPool(std::size_t threadNum)
: _work(std::make_unique<boost::asio::executor_work_guard
    <boost::asio::io_context::executor_type>> 
    (boost::asio::make_work_guard(_service))) //其构造函数只接收一个executor对象
{   
    _threads.reserve(threadNum);
    for(std::size_t i = 0;i < threadNum;i++){
        _threads.emplace_back([this](){
            _service.run();
        }); //启动线程，各线程轮流执行_service.run()
    }
}

boost::asio::io_context& AsioThreadPool::GetIOService(){
    return _service;
}

void AsioThreadPool::stop(){
    _work->reset();  //释放掉对应的io_context，使得io_context.run能够在无任务的情况下直接返回
    //_service.stop();
    for(auto& _ : _threads){
        if(!_.joinable())
            std::cerr << "fuck! thread cannot stop!" << std::endl;
        _.join();
    }
}
