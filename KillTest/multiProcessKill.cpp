#include <iostream>
#include <unistd.h>
#include <signal.h>

int main(){
    pid_t pid[5];
    int i;
    for(i = 0; i < 5;i++){
        pid[i] = fork();
        if(pid[i] == 0) break;
    }

    if(i == 5){
        std::cout << "this is parent" << "\t";
        sleep(3);
        kill(pid[2],SIGKILL);
        std::cout << "kiled\t" << pid[2] << std::endl;
    }else{
        std::cout << " there is " << i + 1 << "th child\t" << "pid:" << getpid() << std::endl;
    }
}