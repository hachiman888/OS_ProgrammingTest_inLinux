#include <iostream>
#include <thread>
#include <shared_mutex>
#include <chrono>
#include <vector>

std::shared_mutex mutex;
int counter = 200;

int main(){
    std::vector<std::thread>thread_pool;
    
    for(int i = 0; i < 5;i++){ //read
        thread_pool.emplace_back(std::thread([](int i){
            while(1){
                mutex.lock_shared();
                std::this_thread::sleep_for(std::chrono::seconds(1));
                std::cout << "------------------i = "<< i << "\ttid = " << std::this_thread::get_id()
                << "\tcounter = " << counter << "\t read" << std::endl;
                mutex.unlock_shared();
                std::this_thread::sleep_for(std::chrono::seconds(2));
            } 
        },i));
    }

    for(int i = 5;i < 7;i++){
        thread_pool.emplace_back(std::thread([](int i){
            while(1){
                mutex.lock();
                std::this_thread::sleep_for(std::chrono::seconds(2));
                counter++;
                std::cout << "------------------i = "<< i << "\ttid = " << std::this_thread::get_id()
                << "\tcounter = " << counter << "\t write" << std::endl;
                mutex.unlock();
                std::this_thread::sleep_for(std::chrono::seconds(3));
            } 
        },i));
    }

    for(auto&& t:thread_pool){
        if(t.joinable())
            t.join();
    }
}