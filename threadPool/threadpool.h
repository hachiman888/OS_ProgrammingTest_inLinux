#pragma once
#include "taskQueue.h"
#include <iostream>
#include <vector>
#include <atomic>
#include <memory>
#include <thread>
#include <chrono>
#include <condition_variable>

template<typename T>
class ThreadPool{
public:
    ThreadPool(uint16_t max,uint16_t min)
    :m_thread_MAXNUM(max),m_thread_MINNUM(min)
    {
        task_Queue_ = std::make_unique<TaskQueue<T>>();
        shutdown_flags.store(false);

        for(int i = 0; i < m_thread_MINNUM;i++){
            working_ThreadArray.emplace_back(std::thread(&ThreadPool::worker_Func,this));
        }

        manager = std::thread(&ThreadPool::manager_Func,this);
    }

    ~ThreadPool(){
        shutdown_flags = true;
        queueNotEmpty.notify_all();//让全部线程自我销毁

        if(manager.joinable())
            manager.join();

        std::lock_guard<std::mutex> lock(tpMutex);
        for(std::thread& elem : working_ThreadArray){
            if(elem.joinable())
                elem.join();
        }
    }

    void addTask(Task<T>&& t){
        if(this->shutdown_flags) return; //若已关闭，直接返回
        
        if(this->task_Queue_->addTask(std::move(t)))
            queueNotEmpty.notify_one();
        else
            std::cout << "加入任务失败!" << std::endl;
        return ;
    }

    uint16_t getBusyNum(){
        return m_busy_threads;
    }

    uint16_t getAliveNum(){
        return m_alive_threads;
    }


private:
    void manager_Func(){
        while(!this->shutdown_flags){
            std::this_thread::sleep_for(std::chrono::seconds(5)); //定时检测一次
            //取出线程池中的任务数和线程数量
            //取出忙碌的线程数量
            size_t queueSize = this->task_Queue_->getSize();
            uint16_t aliveThreadNum = this->m_alive_threads;
            uint16_t busyThreadNum = this->m_busy_threads;

            const uint8_t stepSize = 2;
            //当前任务个数>存活的线程数 && 存活的线程数<最大线程个数,则扩容
            if(queueSize > aliveThreadNum && aliveThreadNum < m_thread_MAXNUM){
                //加锁
                std::lock_guard<std::mutex> locker_t(tpMutex);
                int counter = 0;
                for(int i = 0;i < m_thread_MAXNUM 
                    && aliveThreadNum < m_thread_MAXNUM 
                    && counter < stepSize;i++)
                    {
                        this->working_ThreadArray.emplace_back(std::thread(&ThreadPool::worker_Func,this));
                        m_alive_threads++;
                        counter++;
                    }
            }

            //缩容
            // 忙的线程*2 < 存活的线程数 && 存活的线程>最小线程数
            if(busyThreadNum * 2 < aliveThreadNum && aliveThreadNum > m_thread_MINNUM){
                tpMutex.lock();
                this->m_exit_threads = stepSize;
                tpMutex.unlock();
                for(int i = 0;i < stepSize;i++){
                    queueNotEmpty.notify_one();
                }
            }
        }
        return ;
    }

    void worker_Func(){
        while(true){
            std::unique_lock<std::mutex> locker(tpMutex);
            // 判断任务队列是否为空, 如果为空工作线程阻塞
            while(task_Queue_->getSize() == 0 && this->shutdown_flags != true){
                std::cout << "thread:" << std::this_thread::get_id() << "\twaiting..." << std::endl; 
                queueNotEmpty.wait(locker);
                //解除阻塞后，判断是否需要销毁线程
                if(m_exit_threads > 0){
                    if(m_alive_threads > m_thread_MINNUM){
                        m_alive_threads--;
                        m_exit_threads--;
                        locker.unlock();
                        return ;
                    }
                }
            }
                //判断线程池是否被关闭
                if(this->shutdown_flags == true){
                    locker.unlock();
                    return ;
                }

                //从任务队列中取出一个任务
                auto taskOpt = task_Queue_->takeTask(); 
                //如果taskOpt为std::nullopt,那么下面直接获取taskOpt会抛出致命异常
                if(taskOpt){
                    Task<T> t = std::move(*taskOpt);
                    //忙碌线程+1
                    this->m_busy_threads++;
                    //线程池解锁
                    locker.unlock();
                    //执行任务
                    std::cout << "thread:" << std::this_thread::get_id() << "\tstart working..." << std::endl;
                    t.callback(t.m_arg);
                    //执行完毕，标记arg为nullptr
                    t.m_arg = nullptr;

                    //任务结束
                    std::cout << "thread:" << std::this_thread::get_id() << "\tend working..." << std::endl;
                    this->m_busy_threads--;
                }
        }
        return ;
    }


    std::unique_ptr<TaskQueue<T>> task_Queue_;

    std::atomic<uint16_t>m_alive_threads{0};
    std::atomic<uint16_t>m_busy_threads{0};
    std::atomic<uint16_t>m_exit_threads{0};
    uint16_t m_thread_MAXNUM;
    uint16_t m_thread_MINNUM;
    std::atomic<bool>shutdown_flags; //线程池是否启用的标志

    std::thread manager;
    std::vector<std::thread>working_ThreadArray;

    std::mutex tpMutex;
    std::condition_variable queueNotEmpty;
};