#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <memory>
#include <vector>


class AsioIOServicePool {
public:
    using IOService = boost::asio::io_context;
    using Work = boost::asio::io_context::work;
    using WorkPtr = std::unique_ptr<Work>;

    void Stop();
    boost::asio::io_context& GetIOService();
    static AsioIOServicePool& GetInstance();

    ~AsioIOServicePool();
    AsioIOServicePool(const AsioIOServicePool&) = delete;
    AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;
private:
    AsioIOServicePool(std::size_t threadNum = std::thread::hardware_concurrency());
    std::vector<IOService> _ioServices;
    std::vector<WorkPtr> _works;
    std::vector<std::jthread> _threads;
    std::size_t _nextIOService; //表示需要返回的IOService的索引
};