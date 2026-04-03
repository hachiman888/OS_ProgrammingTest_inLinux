#include "threadpool.h"

void process(int* num){
    std::cout << *num << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
}


int main(){
    ThreadPool<int> threadPool(64,10);
        for(int i = 0;i < 64;i++){
            int* num = new int(i);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            threadPool.addTask(Task<int>(process,num));
        }
}