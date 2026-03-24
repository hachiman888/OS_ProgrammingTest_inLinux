#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>

constexpr int MAX_OPEN = 5000;

void handle_error(const char* msg){
    perror(msg);
    exit(-1);
}

void setPortReuse(int sockfd){
    int opt = 1;
    if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt)) == -1)
    {
        handle_error("setsockopt error!");
    }
    return ;
}

int main(){
    sockaddr_in serverAddr,clientAddr;
    socklen_t clientAddrLen;
    int ret,i,n;
    char buff[BUFSIZ];

    int listenfd = socket(AF_INET,SOCK_STREAM,0);
    if(listenfd == -1)
    {
        handle_error("socket error!");
    }
    
    setPortReuse(listenfd);
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ret = bind(listenfd,reinterpret_cast<sockaddr*>(&serverAddr),sizeof(sockaddr_in));
    if(ret == -1)
    {
        handle_error("bind error!");
    }

    ret = listen(listenfd,128);
    if(ret == -1)
    {
        handle_error("listen error!");
    }

    int efd = epoll_create(MAX_OPEN);
    if(efd == -1)
    {
        handle_error("epoll create error!");
    }

    epoll_event tep,epArr[MAX_OPEN];
    tep.events = EPOLLIN;
    tep.data.fd = listenfd;

    ret = epoll_ctl(efd,EPOLL_CTL_ADD,listenfd,&tep);
    if(ret == -1)
    {
        handle_error("epoll_ctl error!");
    }

    for(;;)
    {
        int nReady = epoll_wait(efd,epArr,MAX_OPEN,-1);
        if(nReady == -1)
        {
            handle_error("epoll_wait error!");
        }

        for(i = 0;i < nReady;i++)
        {
            if(epArr[i].data.fd == listenfd)
            {
                clientAddrLen = sizeof(clientAddr);
                int cfd = accept(listenfd,reinterpret_cast<sockaddr*>(&clientAddr),&clientAddrLen);
                if(cfd == -1)
                {
                    handle_error("accept error!");
                }

                tep.data.fd = cfd;
                tep.events = EPOLLIN;
                ret = epoll_ctl(efd,EPOLL_CTL_ADD,cfd,&tep);
                if(ret == -1)
                {
                    handle_error("epoll_ctl error!");
                }
            }
            else
            {
                int sockfd = epArr[i].data.fd;
                n = read(sockfd,buff,4096);
                if(n < 0)
                {
                    ret = epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,nullptr);
                    close(sockfd);
                    handle_error("read error!");
                }
                else if(n > 0)
                {
                    for(int j = 0;j < n;j++)
                        buff[j] = std::toupper(buff[j]);
                    write(sockfd,buff,n);
                    write(STDOUT_FILENO,buff,n);
                }
                else//n == 0
                {
                    std::cout << "disconnect: "  << sockfd << std::endl;
                    ret = epoll_ctl(efd,EPOLL_CTL_DEL,sockfd,nullptr);
                    close(sockfd); 
                }
            }
        }
    }
}