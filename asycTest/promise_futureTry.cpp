#include <iostream>
#include <future>
#include <chrono>
#include <thread>

void setValue1(std::promise<int>& p){
    p.set_value(10);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    std::cout << "awake " << std::endl;
}

void setValue2(std::promise<int>& p,int){
    p.set_value_at_thread_exit(100);
    std::this_thread::sleep_for(std::chrono::seconds(3));
    std::cout << "awake " << std::endl;
}

int main(){
    std::promise<int> p;

    std::thread t(setValue1,std::ref(p));
    std::future<int> c = p.get_future();
    auto value = c.get();
    std::cout << "value: " << value << std::endl;

    std::packaged_task<int(int)> s([](int x){
        return x+=10;
    });

    std::thread t2(std::ref(s),50);
    c = s.get_future();
    value = c.get();
    std::cout << "value: " << value << std::endl;


    t.join();
    t2.join();
}
     
    