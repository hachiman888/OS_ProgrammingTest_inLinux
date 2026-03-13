#include <iostream>
#include <thread>
#include <future>
#include <utility>
#include <chrono>
#include <unistd.h>
#include <pthread.h>
#include <string>

struct thrd{
    int value;
    char str[256];
};

void sys_err(const char* msg){
    perror(msg);
    exit(-1);
    return ;
}

#if 0 //一坨屎
void* handlefunc(void* arg){
    thrd* t = new thrd();
    t->value = 100;
    std::string s = "hello world";
    std::copy(s.begin(),s.end(),t->str);
    return &t->value;
}

int main(){
    pthread_t tid;
    thrd* temp;
    int ret = pthread_create(&tid,nullptr,handlefunc,nullptr);
    if(ret != 0)
        sys_err("pthread_create error!");

    ret = pthread_join(tid,(void**)&temp);
    std::cout << temp->value << std::endl;
}
#else //cpp现代写法
void handlefunc(std::promise<int>&temp)
{
    std::this_thread::sleep_for(std::chrono::seconds(1));
    temp.set_value(100);
}

int main()
{
    std::promise<int>t;
    std::future<int>value = t.get_future();

    std::thread th(handlefunc,std::ref(t));
    
    if(th.joinable())
        th.join();

    std::cout << value.get() << std::endl; 
}
#endif
