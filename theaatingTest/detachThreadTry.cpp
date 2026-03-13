#if 0
#include <iostream>
#include <thread>
#include <chrono>
#include <future>
#include <utility>

int main(){
    std::promise<int>Obj;
    std::future<int> value = Obj.get_future(); 
    std::thread t([&](std::promise<int>&Obj){
            Obj.set_value(10);
            while(1);
    },std::ref(Obj));
    t.detach();

    std::cout << "main thread return!" << std::endl;
    std::cout << value.get() << std::endl;
}
#else
#include <iostream>
#include <pthread.h>
#include <system_error>
#include <unistd.h>

void* handlefunc(void*){
    sleep(3);
    std::cout << "thread running" << std::endl;
    sleep(3);
    return nullptr;
}

const char* errorHandle(int errn){
    std::system_error ec(errn,std::generic_category());
    return ec.what();
}

int main(){
    pthread_t tid;
    int ret = pthread_create(&tid,nullptr,handlefunc,nullptr);
    if(ret != 0){
        std::cout <<"pthread_create error!"<< errorHandle(ret) << std::endl;
        exit(-1);
    }
    
    ret = pthread_detach(tid);
    if(ret != 0){
        std::cout << "pthread_detach error!"<< errorHandle(ret) << std::endl;
        exit(-1);
    }

    ret = pthread_join(tid,nullptr);
    if(ret != 0){
        std::cout << "pthread_join error!"<< errorHandle(ret) << std::endl;
        exit(-1);
    }

    std::cout << "main thread return!" << std::endl; 
}
#endif