#pragma once
#include "ConnectionManager.h"

class WebSocketServer{
public:
    WebSocketServer(const WebSocketServer&) = delete;
    WebSocketServer& operator=(const WebSocketServer&) = delete;
    WebSocketServer(net::io_context& ioc,uint16_t port);
    void startAccept();   //tcp层面的启动连接
private:

    net::ip::tcp::acceptor _acceptor;
    net::io_context& _ioc;
};
