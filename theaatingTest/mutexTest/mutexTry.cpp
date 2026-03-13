#include <iostream>
#include <system_error>
#include <thread>
#include <mutex>
#include <chrono>

const char* errorHandle(int error_number){
    std::system_error ec(error_number,std::generic_category());
    return ec.what();
}

std::mutex mutex;

void* hanldeFunc(void*){
    while(1){
        mutex.lock();
        std::cout << "hello ";
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "world" << std::endl;
        mutex.unlock();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return nullptr;
}

#if 0
int main(){
    std::thread t([](){
        while(1){
            mutex.lock();
            std::cout << "hello ";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout << "world" << std::endl;
            mutex.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(2));
        }
    });

    while(1){
            mutex.lock();
            std::cout << "HELLO ";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout << "WOLRD" << std::endl;
            mutex.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}
#else
int main(){
    pthread_t tid;
    int ret = pthread_create(&tid,nullptr,hanldeFunc,nullptr);
    if(ret != 0){
        std::cout << errorHandle(ret) << std::endl;
        return -1;
    }

        while(1){
            mutex.lock();
            std::cout << "HELLO ";
            std::this_thread::sleep_for(std::chrono::seconds(2));
            std::cout << "WOLRD" << std::endl;
            mutex.unlock();
            std::this_thread::sleep_for(std::chrono::seconds(2));
    }
}
#endif