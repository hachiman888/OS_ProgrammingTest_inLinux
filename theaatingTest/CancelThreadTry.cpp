#include <iostream>
#include <thread>
#include <chrono>

void threadFunction(std::stop_token stopToken) {
    while (!stopToken.stop_requested()) {
        std::cout << "Thread is running." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
}
    std::cout << "Thread received stop request." << std::endl;
}

int main(){
    std::jthread t(threadFunction);
    std::this_thread::sleep_for(std::chrono::seconds(5));
    t.request_stop();
}