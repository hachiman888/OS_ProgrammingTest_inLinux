#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <chrono>
#include <thread>

template<class T>
class threadSafe_queue{
private:
    std::mutex mtx;
    std::condition_variable cv;
    std::queue<T>queue;

public:
    void threadSafe_push(T value){
        std::lock_guard<std::mutex>lk(mtx);
        this->queue.push(value);
        cv.notify_one();
    }

    T threadSafe_pop(){
        std::unique_lock<std::mutex>lk(mtx);
        cv.wait(lk,[this](){return !queue.empty();});
        T value = std::move(this->queue.front());
        this->queue.pop();
        return value;
    }

    threadSafe_queue() = default;

    threadSafe_queue(const threadSafe_queue&) = delete;
    threadSafe_queue& operator=(const threadSafe_queue&) = delete;

    threadSafe_queue(const threadSafe_queue&&) = delete;
    threadSafe_queue& operator=(const threadSafe_queue&&) = delete;
};


int main(){
    std::mutex forPC;
    threadSafe_queue<int> safeQueue;

    std::thread p([&forPC,&safeQueue]{
        for(int i = 0;;i++){
            safeQueue.threadSafe_push(i);
            {
                std::lock_guard<std::mutex>locker(forPC);
                std::cout << "-----produce-----" << i << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    });

    std::thread c1([&forPC,&safeQueue]{
        for(;;){
            int value = safeQueue.threadSafe_pop();
            {
                std::lock_guard<std::mutex>locker(forPC);
                std::cout << "--------------consume1---" << value << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    });

    std::thread c2([&forPC,&safeQueue]{
        for(;;){
            int value = safeQueue.threadSafe_pop();
            {
                std::lock_guard<std::mutex>locker(forPC);
                std::cout << "--------------consume2---" << value << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    });

    std::thread c3([&forPC,&safeQueue]{
        for(;;){
            int value = safeQueue.threadSafe_pop();
            {
                std::lock_guard<std::mutex>locker(forPC);
                std::cout << "--------------consume3---" << value << std::endl;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }
    });

    p.join();
    c1.join();
    c2.join();
    c3.join();
}