#include "EasySocket.hpp"
#include <sys/wait.h>
#include <signal.h>

void CatchSignal(int sig_num){
    while((waitpid(-1,nullptr,WNOHANG)) > 0);
    return ;
}

void set_reuseport(int sockfd) {     //设置端口复用
    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        std::cerr << "setsockopt reuseport error: " << getting_error_msg(errno) << std::endl;
        exit(-1);
    }
    return ;
}

int main(){
    int ret = 0;
    struct sigaction act;
    act.sa_handler = CatchSignal;
    sigemptyset(&act.sa_mask);
    act.sa_flags = SA_RESTART;
    sigaction(SIGCHLD,&act,nullptr);

    for(int i = 0;i < 10;i++){ //进程池
        pid_t pid = fork();

        if(pid == 0){
            char buf[4096];
            eSocket listenSocket;
            listenSocket.easy_socket(AF_INET,SOCK_STREAM,0);

            int sockfd = listenSocket.getSockFD();
            set_reuseport(sockfd);
            listenSocket.easy_bind(AF_INET,8888,"0.0.0.0");
            listenSocket.easy_listen(128);

            while(1){
                eSocket connectSocket = listenSocket.easy_accept();
                int connectFD = connectSocket.getSockFD();
                // 不该在这退出，应再次等待连接
                if(connectFD == -1){
                    continue;
                }

                while(1){
                    int bytes_record = connectSocket.receive(connectFD,&buf);

                    if(connectFD == -1 || bytes_record <= 0){
                        break;
                    }

                    for(int i = 0; i < bytes_record;i++){
                        buf[i] = std::toupper(buf[i]);
                    }
                    connectSocket.send(connectFD,&buf,bytes_record);
                }
            }
        }  
        else if(pid == -1){
            std::cerr << "fork error!" << getting_error_msg(errno) << std::endl;
            break;
        }
    }
        

    while(1){
        pause();
        }
}


    