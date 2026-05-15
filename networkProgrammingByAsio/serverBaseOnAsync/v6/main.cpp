#include "server_v6.h"
#include "session_v6.h"

int main(){
    asio::io_context ioc;
    Server s(ioc,8888);
    ioc.run();
}