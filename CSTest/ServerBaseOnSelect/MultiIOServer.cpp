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
    constexpr int maxconnect = 1024; 


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
            continue;
        }
        
        if(FD_ISSET(listenfd,&rset)){ //监听到有客户端连接
            if(socketSet.size() >= maxconnect - 3){
                std::cerr << "connections overflow!" << std::endl;
                
                eSocket tempsocket = lSocket.easy_accept();
                if(tempsocket.getSockFD() != -1){
                    close(tempsocket.getSockFD());
                }
                continue;
            }

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

        for(auto it = socketSet.begin();it < socketSet.end();){
            int sockfd = (*it)->getSockFD();
            if(FD_ISSET(sockfd,&rset)){
                int n = read(sockfd,&buf,sizeof(buf));//确保读事件发生，使得read不会阻塞等待客户端消息，立即返回
                
                if(n == -1){
                    std::cerr << "read error!" << getting_error_msg(errno) << std::endl;
                    FD_CLR(sockfd,&allset);
                    close(sockfd);
                    it = socketSet.erase(it);
                    continue; //删除迭代器，不允许其自增，不然会越过一个位置
                }else if(n == 0){ //TODO
                    FD_CLR(sockfd,&allset);
                    close(sockfd);
                    it = socketSet.erase(it);
                    continue;
                }else{
                    for(int j = 0;j < n;j++){
                        buf[j] = std::toupper(buf[j]);
                    }
                    write(sockfd,&buf,n);
                    write(STDOUT_FILENO,&buf,n);
                }

                if(0 == --ret){
                    break;
                }
            }
            it++;
        }
        //TODO
        if(socketSet.empty()){
            maxfd = listenfd;
        }else{
            int newMaxfd = listenfd;
            for(const auto& client : socketSet){
                int fd = client->getSockFD();
                if(newMaxfd < fd)
                    newMaxfd = fd;
            }
            if(maxfd != newMaxfd)
                maxfd = newMaxfd;
        }
    }
}