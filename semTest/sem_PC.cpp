#include <iostream>
#include <thread>
#include <semaphore>
#include <chrono>

const int MAX_NUM = 5;
int queue[MAX_NUM];
std::counting_semaphore<5>blank_sem(5);//初始值为5，最大值为5；
std::counting_semaphore<5>product_sem(0);//初始值为0，最大值为5；

void producer(){
    for(int i = 0;; i = (i+1) % MAX_NUM){
        blank_sem.acquire();
        queue[i] = rand()%900+1;
        std::cout << "-----produced:" << queue[i] <<std::endl;
        product_sem.release(); //生产完之后，给阻塞在此信号量的线程发信号
        std::this_thread::sleep_for(std::chrono::seconds(rand()%1));
    }
}

void consumer(){
    for(int i = 0;;i = (i+1) % MAX_NUM){
        product_sem.acquire(); // 0 时阻塞，等待产品生产
        std::cout << "----------------consumed:" << queue[i] <<std::endl;
        queue[i] = -1;
        blank_sem.release(); //消费完之后，给生产者发信号
        std::this_thread::sleep_for(std::chrono::seconds(rand()%2));
    }
}

int main(){
    srand(time(NULL));
    std::thread p(producer);
    std::thread c(consumer);

    p.join();
    c.join();
}
