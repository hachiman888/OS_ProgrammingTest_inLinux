#include "server_v9.h"
#include "session_v9.h"
#include "AsioThreadPool.h"

std::mutex mutex_quit;
std::condition_variable cond_quit;
bool _b_stop = false;

//26.5.21补充服务器的优雅退出机制
//v10_v2 应用多线程模型来操作同一个ioc
int main(){
    try{
        auto pool = AsioThreadPool::GetInstance(); //启动线程池
        asio::io_context ioc;
        asio::signal_set signals(ioc,SIGINT,SIGTERM); //将想要捕获的信号注册到对应的ioc中，然后ioc将其作为事件注册到对应的事件模型中
        signals.async_wait([&ioc,&pool](auto,auto){ //注册了多少个信号，就要写多少个参数进去
            pool->stop();
            std::unique_lock<std::mutex> _lock(mutex_quit); //此处逻辑是异步逻辑，触发时asio底层会用另一个线程执行任务，所以要加锁
            _b_stop = true;
            cond_quit.notify_one(); //通知主线程解除阻塞
            std::cout << "ioc stopped..." << std::endl;
            ioc.stop();
        });//异步等待，注册信号触发时的回调函数

        std::thread signal_thread([&ioc](){
            ioc.run();
        });

        Server s(pool->GetIOService(),8888); //让服务器使用线程池的ioc，该ioc是与_strand绑定的。相当于子线程
        //无需写ioc.run，已交由线程池中线程调用

        {
            std::unique_lock<std::mutex> _lk(mutex_quit);
            cond_quit.wait(_lk,[](){
                return _b_stop;
            }); //确保主线程阻塞，防止主线程提前结束运行
        }
        
        signal_thread.join();
    }
    catch(std::exception& e){
        std::cerr << "error: " << e.what() << std::endl;
    }
}