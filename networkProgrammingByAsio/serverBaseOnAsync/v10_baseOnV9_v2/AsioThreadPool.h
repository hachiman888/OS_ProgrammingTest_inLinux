#pragma once
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <memory>
#include "Singleton.h"

class AsioThreadPool : public Singleton<AsioThreadPool>
{
    friend class Singleton<AsioThreadPool>;
public:
    boost::asio::io_context& GetIOService();
    void stop();

    ~AsioThreadPool(){}
    AsioThreadPool(const AsioThreadPool&) = delete;
    AsioThreadPool& operator=(const AsioThreadPool&) = delete;

private:
    AsioThreadPool(std::size_t threadNum = std::thread::hardware_concurrency());
    boost::asio::io_context _service;
    std::vector<std::thread> _threads;
    std::unique_ptr<boost::asio::executor_work_guard<boost::asio::io_context::executor_type>> _work;
};
