#include "session_v7.h"
#include "server_v7.h"
#include <iostream>

int main(){
    try{
        asio::io_context io;
        Server s(io,8888);
        io.run(); //如果不写这个，则内核无法自动轮询 触发事件的相关集合
    }
    catch(std::exception& e){
        std::cerr << "error! error is: " << e.what() << std::endl;
    }
    asio::io_context io;
}