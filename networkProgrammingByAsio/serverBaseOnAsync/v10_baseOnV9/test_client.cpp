#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include <chrono>
#include <vector>
#include <mutex>

using namespace std;
using namespace boost::asio::ip;
const int MAX_LENGTH = 1024 * 2;
const int HEAD_LENGTH = 2;
const int HEAD_TOTAL = 4;

std::vector<thread> vec_threads;
std::mutex cout_mtx; // 增加互斥锁保护标准输出

// 线程安全打印函数
void safe_print(const std::string& msg) {
    std::lock_guard<std::mutex> lock(cout_mtx);
    std::cout << msg << std::endl;
}

int main()
{
    // 提前 reserve 避免 vector 扩容导致内存重分配
    vec_threads.reserve(100);

    auto start = std::chrono::high_resolution_clock::now(); 

    for (int i = 0; i < 100; i++) {
        vec_threads.emplace_back([]() {
            try {
                boost::asio::io_context ioc;
                tcp::endpoint remote_ep(address::from_string("127.0.0.1"), 8888);
                tcp::socket sock(ioc);
                boost::system::error_code error = boost::asio::error::host_not_found; 
                
                sock.connect(remote_ep, error);
                if (error) {
                    safe_print("Connect failed, code is " + std::to_string(error.value()) + " msg is " + error.message());
                    return;
                }

                int loop_count = 0;
                while (loop_count < 500) {
                    Json::Value root;
                    root["id"] = 1001;
                    root["data"] = "hello world";
                    std::string request = root.toStyledString();
                    size_t request_length = request.length();

                    if (request_length + HEAD_TOTAL > MAX_LENGTH) {
                        safe_print("Request too large!");
                        break;
                    }

                    char send_data[MAX_LENGTH] = { 0 };
                    
                    // 严格使用 uint16_t (2字节) 处理头部，规避 int 带来的混淆
                    uint16_t msgid = 1001;
                    uint16_t msgid_network = boost::asio::detail::socket_ops::host_to_network_short(msgid);
                    memcpy(send_data, &msgid_network, HEAD_LENGTH);

                    uint16_t request_len_network = boost::asio::detail::socket_ops::host_to_network_short(static_cast<uint16_t>(request_length));
                    memcpy(send_data + HEAD_LENGTH, &request_len_network, HEAD_LENGTH);
                    memcpy(send_data + HEAD_TOTAL, request.c_str(), request_length);

                    boost::asio::write(sock, boost::asio::buffer(send_data, request_length + HEAD_TOTAL));

                    // 接收头部
                    char reply_head[HEAD_TOTAL] = {0};
                    boost::asio::read(sock, boost::asio::buffer(reply_head, HEAD_TOTAL));

                    uint16_t res_msgid = 0;
                    uint16_t res_msglen = 0;
                    memcpy(&res_msgid, reply_head, HEAD_LENGTH);
                    memcpy(&res_msglen, reply_head + HEAD_LENGTH, HEAD_LENGTH);

                    res_msgid = boost::asio::detail::socket_ops::network_to_host_short(res_msgid);
                    res_msglen = boost::asio::detail::socket_ops::network_to_host_short(res_msglen);

                    // 接收体
                    if (res_msglen >= MAX_LENGTH) {
                        safe_print("Response message length exceeds buffer!");
                        break;
                    }

                    char msg[MAX_LENGTH] = { 0 };
                    size_t msg_length = boost::asio::read(sock, boost::asio::buffer(msg, res_msglen));

                    Json::Reader reader;
                    if (reader.parse(std::string(msg, msg_length), root)) {
                        // 减少高频打印，或者格式化后用安全打印
                        // safe_print("msg id is " + std::to_string(root["id"].asInt()) + " msg is " + root["data"].asString());
                    }
                    
                    loop_count++;
                }
            }
            catch (std::exception& e) {
                safe_print(std::string("Exception: ") + e.what());
            }
        });
        
        // 压测时 1 秒创建一个线程可能有点慢（总共需要100秒），可以根据压测需求缩短这个时间
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); 
    }

    for (auto& t : vec_threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    auto end = std::chrono::high_resolution_clock::now(); 
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start); 
    std::cout << "Time spent: " << duration.count() << " seconds." << std::endl; 
    
    return 0;
}