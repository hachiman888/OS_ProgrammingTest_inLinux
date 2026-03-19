#include <iostream>
#include "../EasySocket.hpp"
#include <vector>
#include <sys/select.h>
#include <memory>

void reUsePort(int sockfd){
    int opt = 1;
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEPORT,std::bit_cast<void*>(&opt),sizeof(opt)) == -1){
        std::cerr << "setsockopt error!" << getting_error_msg(errno) << std::endl;
        exit(-1);
    }
    return ;
}

int main(){
    eSocket lSocket;
    int ret;
    char buf[BUFSIZ];
    std::vector<std::unique_ptr<eSocket>>socketSet;


    lSocket.easy_socket(AF_INET,SOCK_STREAM,0);
    reUsePort(lSocket.getSockFD());
    lSocket.easy_bind(AF_INET,8888,"0.0.0.0");
    lSocket.easy_listen(128);

    int listenfd = lSocket.getSockFD();
    fd_set allset,rset;
    FD_ZERO(&allset);
    FD_SET(listenfd,&allset);
    int maxfd = listenfd;

    while(1){
        rset = allset;
        ret = select(maxfd+1,&rset,nullptr,nullptr,nullptr);
        
        if(ret < 0){
            std::cerr << "select error!" <<getting_error_msg(errno) << std::endl;
            exit(-1);
        }
        
        if(FD_ISSET(listenfd,&rset)){ //监听到有客户端连接
            auto cSocket = std::make_unique<eSocket>(lSocket.easy_accept());
            
            if(cSocket->getSockFD() == -1){
                std::cerr << "Accept failed, continuing..." << std::endl;
                continue;
            }

            int connfd = cSocket->getSockFD();
            FD_SET(connfd,&allset);

            if(maxfd < connfd)
                maxfd = connfd;
            
            socketSet.push_back(std::move(cSocket));
            
            if(0 == --ret){
                continue;
            }
        }

        for(int i = listenfd+1;i <= maxfd;i++){
            if(FD_ISSET(i,&rset)){
                int n = read(i,&buf,sizeof(buf));
                
                if(n == -1){
                    std::cerr << "read error!" << getting_error_msg(errno) << std::endl;
                    close(i);
                    FD_CLR(i,&allset);
                }else if(n == 0){ //TODO
                    close(i);
                    FD_CLR(i,&allset);
                }else{
                    for(int j = 0;j < n;j++){
                        buf[j] = std::toupper(buf[j]);
                    }
                    write(i,&buf,n);
                    write(STDOUT_FILENO,&buf,n);
                }

            }
        }
    }
}