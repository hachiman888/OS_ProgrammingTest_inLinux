#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <chrono>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/reader.h>
#include <jsoncpp/json/value.h>

namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = net::ip::tcp;

namespace my_program_state{
    std::size_t request_count(){  //用于统计有多少请求
        static std::size_t count = 0;
        return ++count;
    }

    std::time_t now(){
        return std::time(0);//返回当前时间戳
    }
}

class Http_Connection : public std::enable_shared_from_this<Http_Connection>
{
public:
    Http_Connection(tcp::socket socket):_socket(std::move(socket)){}
    
    void start(){
        read_Request();     //开始处理请求
        check_deadline();   //超时检测机制，防止连接处理超时
    }

private:
    tcp::socket _socket;
    beast::flat_buffer _buffer{8192}; //beast提供的一个扁平缓冲区
    http::request<http::dynamic_body> _request; //beast库提供的一个请求体的格式，dynamic body支持各种类型的请求（文本，json，html，font等类型）
    //还有http::string_body,或http::binary_body
    http::response<http::dynamic_body> _response; //回应也用dynamic_body
    net::steady_timer _deadline{_socket.get_executor(),std::chrono::seconds(60)}; //设置一个时钟，其与_socket的调度器绑定，每60秒调度一次
    
    void read_Request(){
        auto self = shared_from_this(); // 同理
        http::async_read(_socket,_buffer,_request, 
            [self](beast::error_code ec,std::size_t bytes_transferred){ //该函数对回调函数的参数列表和返回值有要求，可以自行查阅声明
                boost::ignore_unused(bytes_transferred);//可以将实际不用的变量丢进去，避免警告
                if(!ec){
                    self->process_Request();
                }
            });
    }

    void check_deadline(){
        auto self = shared_from_this(); //使得该类对象指针引用计数+1，防止http_connection对象提前析构，导致lambda表达式使用悬空引用
        _deadline.async_wait([self](boost::system::error_code ec){  //此处若用this，若http_connection对象异常析构，则直接悬空引用
            self->_socket.close(ec);
        });
    }

    void process_Request(){
        _response.version(_request.version());  //将回复报文的http版本与请求报文的http版本绑定，使他们自动相同
        _response.keep_alive(false);    //短连接，true为长连接
        switch (_request.method()) //用于判断它是非法请求类型，或是get/post
        {
        case http::verb::get:
            _response.result(http::status::ok);
            _response.set(http::field::server,"beast");
            createResponse();
            break;
        case http::verb::post:
            _response.result(http::status::ok);
            _response.set(http::field::server,"beast");
            createPostResponse();
            break;
        default:
            _response.result(http::status::bad_request); //设置回复报文的状态码，状态为bad_request
            _response.set(http::field::content_type,"text/plain"); //设置回复报文类型为纯文本类型，更多的自行查阅官网文档
            beast::ostream(_response.body()) << "Invaild request-method '" //beast库中ostream用于向某个对象内写数据 
                << std::string(_request.method_string()) << "'"; 
            break;
        }

        //处理了请求，并设置完回复报文之后，发送回复报文给对端
        write_Response();
    }

    void createResponse(){
        if(_request.target() == "/count"){  //判断要访问的路由是否是服务器所能提供的
            _response.set(http::field::content_type,"text/html");
             beast::ostream(_response.body()) << "<html>\n"
                << "<head><title>Request count</title></head>\n"
                << "<body>\n"
                << "<h1>Request count</h1>\n"
                << "<p>There have been "
                << my_program_state::request_count()
                << " requests so far.</p>\n"
                << "</body>\n"
                << "</html>\n";
            //创建html格式的回复报文
        }
        else if(_request.target() == "/time"){
            _response.set(http::field::content_type,"text/html");
             beast::ostream(_response.body())
                << "<html>\n"
                << "<head><title>Current time</title></head>\n"
                << "<body>\n"
                << "<h1>Current time</h1>\n"
                << "<p>The current time is "
                << my_program_state::now()
                << " seconds since the epoch.</p>\n"
                << "</body>\n"
                << "</html>\n";
        }
        else //若没找到，则404 not found
        {
            _response.result(http::status::not_found);  //设置回复报文的状态码，其状态为404 not found
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body()) << "File not found\r\n";
        }
    }

    void createPostResponse(){
        if(_request.target() == "/email"){
            auto& body = _request.body(); // 先取出包体
            auto body_str = beast::buffers_to_string(body.data()); //包体内容转换成string格式,一般会发序列化后的json报文
            std::cout << "receive body is " << body_str << std::endl;
            _response.set(http::field::content_type,"text/json"); //打印完后，返回回复报文
            Json::Value root;
            Json::Reader reader;
            Json::Value src_root;
            bool parse_success = reader.parse(body_str,src_root); //将body_str反序列化，存进src_root
            if(!parse_success){
                //若解析失败，则返回一个错误码
                std::cout << "Failed to parse json data..." << std::endl;
                root["error"] = 1001;
                std::string jsonstr = root.toStyledString();
                //写入回复报文包体中
                beast::ostream(_response.body()) << jsonstr;
                return;
            }

            //若未发送反序列化失败，则取出发过来的email，处理掉
            auto email = src_root["email"].asString();
            std::cout << "email is " << email << std::endl;
            root["error"] = 0;
            root["email"] = src_root["email"];
            root["msg"] = "receive email post success";
            std::string jsonstr = root.toStyledString();
            beast::ostream(_response.body()) << jsonstr; //写入回复报文包体中
        }else{
            _response.result(http::status::not_found);  //设置回复报文的状态码，其状态为404 not found
            _response.set(http::field::content_type, "text/plain");
            beast::ostream(_response.body()) << "File not found\r\n";
        }
    }

    void write_Response(){
        auto self = shared_from_this(); //同理
        _response.content_length(_response.body().size());  //设置回复报文的包体长度
        http::async_write(_socket,_response,[self]
            (beast::error_code ec,std::size_t bytes_transferred){
                self->_socket.shutdown(tcp::socket::shutdown_send,ec);   //发送完成后，服务器主动断开发送端
                //仅可以主动断开发送端，不然会造成time wait状态，引起大量僵尸连接，详细可问deepseek
                self->_deadline.cancel(); //发送完毕后，主动停止定时器
            });
    }
};

// void http_Server(tcp::acceptor& acceptor,tcp::socket& socket){
//     acceptor.async_accept(socket,[&](beast::error_code ec){
//         if(!ec){
//             std::make_shared<Http_Connection>(std::move(socket))->start(); //此处即便创建临时变量，Http_Connection对象也不会提前析构，因为引用计数保证了生命周期与回调函数一致
//         }
//         //无论是否出现错误，都要再次监听,当前模式是短连接，客户端发完一段消息则断开连接
//         http_Server(acceptor,socket);
//     });
// }
//以上方法违反移动思想，极其反人类，且多线程中极其危险

// 1. 去掉 tcp::socket& 参数，只需要传 acceptor
void http_Server(tcp::acceptor& acceptor) {
    // 2. 显式捕获 acceptor（推荐，比 [&] 更安全）
    // 3. 在回调参数中直接接收 Asio 为我们创建的新 peer_socket
    //在现代的 Boost 版本（1.70 及以上）中，async_accept 提供了一个更优雅的重载：
    //它不再需要你从外部传入一个 socket，而是直接在回调函数中吐出一个构造好的 socket。
    acceptor.async_accept(
        [&acceptor](beast::error_code ec, tcp::socket peer_socket) {
            if (!ec) {
                // 4. 将新生成的 peer_socket move 给 Http_Connection
                std::make_shared<Http_Connection>(std::move(peer_socket))->start(); 
            }
            // 5. 递归监听下一个连接
            http_Server(acceptor);
        });
}

int main(){
    try{
        boost::asio::io_context ioc;
        auto const address = boost::asio::ip::make_address_v4("127.0.0.1");
        tcp::acceptor acceptor(ioc,{address,8080});
        http_Server(acceptor);
        ioc.run();
    }catch(std::exception& e){
        std::cerr << "exception is " << e.what() << std::endl;
        return EXIT_FAILURE;   
    }
}