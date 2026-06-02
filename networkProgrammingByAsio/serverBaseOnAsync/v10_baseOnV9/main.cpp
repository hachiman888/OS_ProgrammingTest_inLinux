#include "server_v9.h"
#include "session_v9.h"
#include "AsioIOServicePool.h"

//26.5.21补充服务器的优雅退出机制

int main(){
    try{
        auto pool = AsioIOServicePool::GetInstance();
        asio::io_context ioc;
        asio::signal_set signals(ioc,SIGINT,SIGTERM); //将想要捕获的信号注册到对应的ioc中，然后ioc将其作为事件注册到对应的事件模型中
        signals.async_wait([&ioc](auto,auto){ //注册了多少个信号，就要写多少个参数进去
            ioc.stop();
            std::cout << "ioc stopped..." << std::endl;
        });//异步等待，注册信号触发时的回调函数

        Server s(ioc,8888);
        ioc.run();
    }
    catch(std::exception& e){
        std::cerr << "error: " << e.what() << std::endl;
    }
}