#include <iostream>
#include <unistd.h>
#include <sys/time.h>
#include <signal.h>

void handleSignal(int){
    std::cout << "hello world" << std::endl;
}

int main(){
    struct itimerval test;
    test.it_interval = {2,0};
    test.it_value = {1,0};
    
    int ret = setitimer(ITIMER_REAL,&test,nullptr);
    if(ret == -1){
        perror("setitimer error!"); 
    }
    signal(SIGALRM,handleSignal);
    while(1){}
}