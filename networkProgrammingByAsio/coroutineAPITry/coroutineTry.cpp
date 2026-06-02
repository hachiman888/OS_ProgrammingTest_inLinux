#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>

using boost::asio::ip::tcp;
using boost::asio::awaitable; //awaitable 允许异步等待，即把协程挂起，主线程继续向下执行
using boost::asio::co_spawn;  //boost启动协程的关键字
using boost::asio::detached;  //启动协程的方式，detached算其中一种，即启动一个协程，默认让它在后台执行，让它与主线程处于分离状态
using boost::asio::use_awaitable; //想让协程可以异步地等待某函数返回，得使用这个（传入函数的参数列表）
namespace this_coro = boost::asio::this_coro; //返回当前协程所执行的环境

awaitable<void> echo(tcp::socket socket){
    //对于网络，必须使用try-catch块来提升程序健壮性
    try{
        char data[1024];
        std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data),use_awaitable); //要求其以协程的方式调用，允许函数等待，而非直接返回
        co_await boost::asio::async_write(socket,boost::asio::buffer(data,n),use_awaitable); //要求其以等待可写的事件触发，若事件未触发，则协程挂起，转让控制权
    }catch(std::exception& e){
        std::cerr << "echo error!Exception is " << e.what() << std::endl;
    }
}

awaitable<void> listener(){ //为了使协程可以调用此函数，需要将此函数定义为可异步等待的
    auto executor = co_await this_coro::executor; //通过co_await的方式，异步地获取调用这个函数的协程的调度器，如果一直无返回，co_await会先让这个协程挂起
    tcp::acceptor acceptor(executor,{tcp::v4(),8888});   //将acceptor与调度器绑定，后序acceptor.async_wait会将事件交由调度器处理
    for(;;){
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable); //通过co_await的方式，异步地获取该函数的返回值，如果暂时没有，则协程主动挂起，转让控制权
        co_spawn(executor,echo(std::move(socket)),detached); //再启动一个协程来处理数据,move完该作用域socket作废
    }
}

int main(){
    try{
        boost::asio::io_context ioc(1);
        boost::asio::signal_set signals(ioc,SIGINT,SIGTERM);
        signals.async_wait([&ioc](auto,auto){
            ioc.stop();
        });

        co_spawn(ioc.get_executor(),listener(),detached); 
        //启动一个协程，该协程与ioc的调度器绑定，协程调用listener函数作为任务，以分离的方式启动（启动完继续向下执行），无法获取结果或异常
        //不能用 co_await 来等它结束或拿返回值
        ioc.run(); //向下执行到这，调度器开始调度协程
    }
    catch(std::exception& e){
        std::cerr << "Exception is " << e.what() << std::endl;
    }
}