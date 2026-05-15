#include <iostream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>
#include <string>
#include <thread>
#include <boost/asio.hpp>
#include <chrono>
using namespace std;

//先将要发送的信息体序列化，然后再对消息长度进行字节序转换
const int MAX_LENGTH = 2048;
const int HEAD_LENGTH = 2;

int main(){
    try{
        boost::asio::io_context ioc;
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string("127.0.0.1"),8888);
        boost::asio::ip::tcp::socket socket(ioc);
        boost::system::error_code ec = boost::asio::error::host_not_found;
        socket.connect(ep,ec);
        if(ec){
            cout << "connect failed, code is " << ec.value() << " error msg is " << ec.message();
		    return -1;
        }
        
        std::thread sendMission([&socket](){
            for(int i = 0;;i++){
                this_thread::sleep_for(std::chrono::milliseconds(200));
                Json::Value root;
                root["id"] = i;
                root["data"] = "hello world!";
                std::string msg = root.toStyledString();
                std::size_t request_len = msg.size();

                char send_data[MAX_LENGTH] = { 0 };
                auto length = boost::asio::detail::socket_ops::host_to_network_short(request_len);
                memcpy(send_data,&length,2);
                memcpy(send_data + 2,msg.c_str(),request_len);
                boost::asio::write(socket,boost::asio::buffer(send_data,request_len + 2));
            }
        });

        std::thread readMission([&socket](){
            for(;;){
                char recv_data[MAX_LENGTH] = { 0 };
                boost::asio::read(socket,boost::asio::buffer(recv_data,2));
                short length;
                memcpy(&length,recv_data,2);
                short msg_len = boost::asio::detail::socket_ops::network_to_host_short(length);
                memset(recv_data,0,MAX_LENGTH);
                boost::asio::read(socket,boost::asio::buffer(recv_data,msg_len));
                Json::Reader reader;
                Json::Value root;
                reader.parse(std::string(recv_data,msg_len),root);
                std::cout << "msg id is: " << root["id"].asInt() 
                    << "\tmsg data is: " << root["data"].asString()
                    << std::endl;  
            }
        });

        sendMission.join();
        readMission.join();
    }
    catch(std::exception& e){
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}