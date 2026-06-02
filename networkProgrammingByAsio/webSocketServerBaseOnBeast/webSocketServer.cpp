#include"webSocketServer.h"

WebSocketServer::WebSocketServer(net::io_context& ioc,uint16_t port)
:_ioc(ioc),_acceptor(ioc,net::ip::tcp::endpoint(net::ip::tcp::v4(),port))
{
    std::cout << "Server started on port " << port <<std::endl;
}

void WebSocketServer::startAccept(){
    auto con_ptr = std::make_shared<Connection>(_ioc);

    _acceptor.async_accept(con_ptr->GetSocket(),
    [this,con_ptr](error_code ec){
        try{
            if(!ec){
                con_ptr->AsyncAccept(); //tcp建立成功，调用websocket的accept，协议升级
            }else{
                std::cerr << "async accept error is" 
                    << ec.what() << std::endl;
            }
        startAccept();  //处理完一个连接后，再次监听端口
        }catch(std::exception& e){
            std::cerr << "async accept exception is" 
                << e.what() << std::endl;
        }
    });
}