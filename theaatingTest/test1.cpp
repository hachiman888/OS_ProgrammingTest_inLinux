#include <thread>
#include <iostream>
#include <unistd.h>
#include <pthread.h>

void sys_err(const char* msg){
    perror(msg);
    exit(-1);
    return ;
}

void* handleFunc(void* arg){
    std::cout << "hello world" << std::endl;
    std::cout << "my tID:\t" << pthread_self() << "\tmy PID:" << getpid() << std::endl;
    return nullptr;
}

#if 0
int main(){
    std::thread t([](){
        std::cout << "hello world!" << std::endl;
        std::cout << "my tID:\t" << std::this_thread::get_id() << "\tmy PID:" << getpid() << std::endl;
    });

    if(t.joinable())
        t.join();
    std::cout << "parent tID:\t" << std::this_thread::get_id() << "\tparent PID:" << getpid() << std::endl;
}
#else
int main(){
    pthread_t tid;
    int ret = pthread_create(&tid,nullptr,handleFunc,nullptr);
    if(ret != 0)
        sys_err("pthread_create error!");
    pthread_join(tid,nullptr);

    std::cout << "parent tID:\t" << pthread_self() << "\tparent PID:" << getpid() << std::endl;
}
#endif