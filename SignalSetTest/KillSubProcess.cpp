#include <iostream>
#include <signal.h>
#include <sys/wait.h>

void sys_err(const char* msg)
{
    perror(msg);
    exit(-1);
    return ;
}

void catchSignal(int signum)
{   
    pid_t wpid;
    int status;
    while((wpid = waitpid(-1,&status,0))!= -1){
        if(WIFEXITED(status))
            std::cout << "child had been killed\t" << "pid:" << wpid
            << "\tstatus:" << WEXITSTATUS(status) << std::endl;
    }
    return ;
}

int main(){
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set,SIGCHLD);
    sigprocmask(SIG_BLOCK,&set,nullptr);//注册阻塞
    int i;

    for(i = 0;i < 10;i++){
        pid_t pid = fork();
        if(pid == 0)
            break;
    }

    if(i == 10){
        struct sigaction act;
        act.sa_handler = catchSignal;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        sigaction(SIGCHLD,&act,nullptr);
        sigprocmask(SIG_UNBLOCK,&set,nullptr);//解除阻塞

        std::cout << "i'm parent,pid = " << getpid() << std::endl;
        while(1);

    }else{
        std::cout << "i'm child,pid = " << getpid() << std::endl;
        return i;
    }
}