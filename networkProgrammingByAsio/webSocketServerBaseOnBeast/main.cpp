#include "webSocketServer.h"

int main(){
    try{
        net::io_context ioc;
        WebSocketServer s(ioc,8888);
        s.startAccept();
        ioc.run();
    }catch(std::exception& e){
        std::cerr << "main exception is " << e.what()
            << std::endl;
    }
}