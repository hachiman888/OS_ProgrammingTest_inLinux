#include <iostream>
#include <future>
#include <chrono>
#include <thread>

int main(){
    std::future<int>msg = std::async(std::launch::async,[](int x){
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return x+10;
    },30);
    
    std::cout << "signal value:" << msg.get() << std::endl;
}