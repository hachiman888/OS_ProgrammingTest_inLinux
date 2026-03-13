#include "EasySocket.hpp"

int main(){
    eSocket client;
    client.easy_socket(AF_INET,SOCK_STREAM,0);
    sockaddr_in serverAddr;

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(8888);
    serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    client.easy_connect(serverAddr);
    while(1){
        std::string msg = "nmsl";
        client.send(client.getListenFD(),msg);
        sleep(3);
        client.receive(client.getListenFD());
    }
}