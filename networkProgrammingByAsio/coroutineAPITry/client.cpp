#include <boost/asio.hpp>
#include <iostream>
#include <string>
const int MAX_LENGTH = 1024;

int main(){
    try{
        boost::asio::io_context ioc;
        boost::asio::ip::tcp::endpoint remote_point(boost::asio::ip::make_address_v4("0.0.0.0"),8888);
        boost::asio::ip::tcp::socket socket(ioc);
        boost::system::error_code ec = boost::asio::error::host_not_found;
        socket.connect(remote_point,ec);
        if(ec){
            std::cerr << "connect error! error code is " << ec.value() 
                << "error message: " << ec.message() << std::endl;
            return -1; 
        }

        char data[MAX_LENGTH];
        auto n = boost::asio::write(socket,boost::asio::buffer("Hello World!"));
        std::cout << "have written 'hello world to server!'" << std::endl;

        boost::asio::read(socket,boost::asio::buffer(data,n));
        std::cout << "server reply is " << std::string(data,n) << std::endl;

    }
    catch(std::exception& e){
        std::cerr << "exception is " << e.what() << std::endl;
    }
}