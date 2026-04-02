#pragma once
#include <functional>
#include <mutex>
#include <array>
#include <optional>

constexpr uint32_t QUEUE_MAX = 8;

template<typename T>
class ThreadPool;

template<class T>
class Task{
public:
    std::function<void(T*)>callback; //function不可拷贝，只可移动
    T* m_arg;

    template<typename F>
    Task(F&& f,T* args):callback(std::forward<F>(f)),m_arg(args){}

    Task():callback(nullptr),m_arg(nullptr){}

    Task(Task&& other) noexcept 
    : callback(std::move(other.callback)),m_arg(other.m_arg){
        other.m_arg = nullptr;
    }
    
    Task& operator=(Task&& other) noexcept{
        if(this != &other){
            this->callback = std::move(other.callback);
            this->m_arg = other.m_arg;
            other.m_arg = nullptr;
        }
        return *this;
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    ~Task() = default;
};

template<typename T>
class TaskQueue{
template<typename U>
    friend class ThreadPool;
public:
    TaskQueue() = default;
    ~TaskQueue() = default;

    bool addTask(Task<T>&& t){
        std::lock_guard<std::mutex> locker(queue_mutex_);
        if(size_ >= QUEUE_MAX) return false;

        queue_[tail_] = std::move(t);
        tail_ = (tail_ + 1) % QUEUE_MAX;
        size_++;
        return true;
    }
    
    bool addTask(std::function<void(T*)>func,T* arg){
        return addTask(Task(std::move(func),arg));
    }
    
    std::optional<Task<T>> takeTask(){
        std::lock_guard<std::mutex> locker(queue_mutex_);
        if(size_ == 0) return std::nullopt;

        Task<T> result = std::move(queue_[head_]);
        head_ = (head_ + 1) % QUEUE_MAX;
        size_--;
        return result; //禁止拷贝后，Task会尝试移动，且optional类可以接收右值
    }

    size_t getSize(){
        std::lock_guard<std::mutex> lg(queue_mutex_);
        return this->size_;
    }

    
private:
    std::mutex queue_mutex_;
    std::array<Task<T>,QUEUE_MAX> queue_;
    size_t head_ = 0;
    size_t tail_ = 0;
    size_t size_ = 0;
};