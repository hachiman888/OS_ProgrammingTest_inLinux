#include <iostream>
#include <unistd.h>

int main(){
    int i = 0; 
    for(;i < 5;i++){
        if(fork() == 0) break;//如果不这样写，子进程也会创建新的子进程，使得最终创建了2^n-1个进程
    }
    if(i == 5){
        sleep(5);
        std::cout << " I am the parent" << std::endl;
    }
    else{
        sleep(i);
        std::cout << " i am the " << i + 1 << "th son" << std::endl;
    }
}
