#include <iostream>
#include <thread>
#include <unistd.h>
#include <pthread.h>
#include <vector>

void sys_err(const char* msg){
    perror(msg);
    exit(-1);
    return ;
}

void* handlefunc(void* arg){
    int* p = static_cast<int*>(arg);//处理void*指针
    int i = *p;//解引用获得数据
    sleep(i+1);
    std::cout << "i'm the " << i+1 << "th thread" 
            << "\tmy tID:\t" << pthread_self() 
            << "\tmy PID:" << getpid() << std::endl;
    return nullptr;  
}

#if 0
int main(){
    std::vector<std::thread>t;
    for(int i = 0;i < 5;i++){
        t.push_back(std::thread([](int arg){
            sleep(arg);
            std::cout << "i'm the " << arg << "th thread" 
            << "\tmy tID:\t" << std::this_thread::get_id() << "\tmy PID:" << getpid() << std::endl;  
        },i+1));
    }
    for(auto&& elem:t){
        if(elem.joinable())
            elem.join();
    }

    std::cout << "parent tID:\t" << std::this_thread::get_id() << "\tparent PID:" << getpid() << std::endl;
}
#else
int main(){
    pthread_t tid;
    int ret,i;
    std::vector<int>nums(5);

    for(i = 0;i < 5;i++){
        nums[i] = i;
        ret = pthread_create(&tid,nullptr,handlefunc,&nums[i]);//传入地址
        if(ret != 0)
            sys_err("pthread_create error!");
    }

    sleep(i+1);
    std::cout << "parent tID:\t" << pthread_self() << "\tparent PID:" << getpid() << std::endl;
}
#endif