#include <iostream>
#include "../include/Server.h"
#include "../include/AsioIOServicePool.h"
#include <boost/asio/signal_set.hpp>

int main(){
    try{
        auto& pool = AsioIOServicePool::GetInstance();
        boost::asio::io_context ioc;
        boost::asio::signal_set signals(ioc,SIGINT,SIGTERM);
        signals.async_wait([&ioc,&pool](auto,auto){
            ioc.stop();
            pool.Stop();
        });

        Server s(ioc,8888); //md这里写了个临时对象，导致服务器立马被析构
        ioc.run();
    }catch(std::exception& e){
        std::cerr << "exception is " << e.what() << std::endl;
    }
}