#include "EasySocket.hpp"
#include <thread>
#include <vector>

//constexpr uint8_t MAX_THREAD_NUMBER = 256;

void set_reuseport(int sockfd) {     //设置端口复用
    int opt = 1;
    if(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt)) == -1) {
        std::cerr << "setsockopt reuseport error: " << getting_error_msg(errno) << std::endl;
        exit(-1);
    }
    return ;
}

//父线程只负责启动线程和回收线程
int main(){
    std::vector<std::thread>thread_pool;

    for(int i = 0; i < 10;i++){
        thread_pool.emplace_back(std::thread([](){
            char buf[4096];
            eSocket listenSocket;
            listenSocket.easy_socket(AF_INET,SOCK_STREAM,0);
            
            int sockfd = listenSocket.getSockFD();
            set_reuseport(sockfd);
            listenSocket.easy_bind(AF_INET,8888,"0.0.0.0");
            listenSocket.easy_listen(128);

            for(;;){
                eSocket connSocket = listenSocket.easy_accept();
                int connFD = connSocket.getSockFD();
                if(connFD == -1){
                    continue;
                }

                while(1){
                    int n = connSocket.receive(connFD,&buf);
                    
                    if(connFD == -1 || n <= 0){
                        break;
                    }

                    connSocket.send(connFD,&buf,n);
                }
            }
        }));
    }

    for(auto& thread : thread_pool){
        thread.detach();    
    }
    pause();
}