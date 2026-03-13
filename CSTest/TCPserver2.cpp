#include "EasySocket.hpp"

int main(){
    eSocket socket;
    socket.easy_socket(AF_INET,SOCK_STREAM,0);
    socket.easy_bind(AF_INET,8888,"0.0.0.0");
    socket.easy_listen(128);
    socket.easy_accept();
    while(1){
        socket.receive(socket.getConnectFD());
        socket.send(socket.getConnectFD());
    }
}