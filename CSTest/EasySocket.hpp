//ONLY FOR CONCURRENCY
#pragma once
#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <system_error>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <cstring>

std::string getting_error_msg(int err_num){
    std::error_code ec(err_num,std::generic_category());
    return ec.message();
}

class eSocket{
private:
    int sockfd;

public:
    eSocket():sockfd(-1){}
    explicit eSocket(int fd):sockfd(fd){}
    
    eSocket(eSocket&& other):sockfd(other.sockfd){
        other.sockfd = -1;
    }
    eSocket& operator=(eSocket&& other){
        if(this->sockfd != other.sockfd && other.sockfd != -1){
            this->sockfd = other.sockfd;
            other.sockfd = -1;
        }
        return *this;
    }

    eSocket(const eSocket& other) = delete;
    eSocket& operator=(const eSocket& other) = delete;

    ~eSocket(){
        if(sockfd != -1) close(sockfd);
    }

    int getSockFD() const {
        return this->sockfd;
    }

    void easy_socket(int domain,int type,int protocol){
        this->sockfd = socket(domain,type,protocol);
        if(sockfd == -1){
            std::cerr <<"socket error!"<< getting_error_msg(errno) << std::endl;
            exit(-1);
        }
    }

    void easy_bind(const int type,int port_num,const std::string ip_addr){
        sockaddr_in serverAddr;
        memset(&serverAddr,0,sizeof(sockaddr_in));
        serverAddr.sin_family = type;
        serverAddr.sin_port = htons(port_num);
        
        if (ip_addr == "0.0.0.0") {
            serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        } else {
            if (inet_pton(type, ip_addr.c_str(), &serverAddr.sin_addr) <= 0) {
                std::cerr << "inet_pton error! Invalid IP address" << std::endl;
                exit(-2);
            }
        }

        int ret = bind(this->sockfd,
            std::bit_cast<sockaddr*>(&serverAddr),sizeof(sockaddr_in));
        //或者使用reinterpret_cast
        if(ret == -1){
            std::cerr << "bind error!" << getting_error_msg(errno) << std::endl;
            exit(-2);
        }       
    }

    void easy_listen(int num){
        int res = listen(this->sockfd,num);
        if(res == -1){
            std::cerr << "listen error!" << getting_error_msg(errno) << std::endl;
            exit(-3);
        }
    }

    eSocket easy_accept(){
        int connectFD = accept(this->sockfd,nullptr,nullptr);
        if(connectFD == -1){
            std::cerr << "accept error!" << getting_error_msg(errno) << std::endl;
            return eSocket(-1);
        }
        return eSocket(connectFD);
    }

    /*
    void send(int sfd,std::string& msg){
        int rec = write(sfd,msg.c_str(),msg.size());
        if(rec == -1){
            std::cerr << "write error!" << getting_error_msg(errno) << std::endl;
            exit(-6);
        }
    }
    */

    void send(int sfd,void* buf,int count){
        if(sfd < 0 || buf == nullptr || count <= 0) return ;

        int rec = write(sfd,buf,count);
        if(rec == -1){
            std::cerr << "write error!" << getting_error_msg(errno) << std::endl;
        }
        write(STDOUT_FILENO,buf,count);
    }

    //to do
    int receive(int& sfd,void* buf){
        if(sfd < 0 || buf == nullptr) return -1;

        int ret = read(sfd,buf,4096);
        if(ret == -1){
            std::cerr << "read error!" << getting_error_msg(errno) << std::endl;
            close(sfd);
            sfd = -1;
            return -1;
        }else if(ret == 0){
            close(sfd);
            sfd = -1;
            return 0;
        }
        /*
        for(int i = 0;i < count;i++){
            buf[i] = std::toupper(buf[i]);
            std::cout << buf[i];
        }
        std::cout << std::endl;
        */
       return ret;
    }
};