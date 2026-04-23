#include "session_v3.h"
#include "server_v3.h"
#include <iostream>

int main(){
    asio::io_context io;
    Server s(io,8888);
    io.run(); //如果不写这个，则内核无法自动轮询 触发事件的相关集合
}