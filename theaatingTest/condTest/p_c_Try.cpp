#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <unistd.h>

std::condition_variable cv;
std::mutex m1;

class listNode{
public:    
    listNode* next;
    int num;

    listNode():next(nullptr),num(0){}

    ~listNode() = default;
};

int main(){
    listNode* head = nullptr;
    srand(time(NULL));

    std::thread consumuer([&head](){
        listNode* mp;
        for(;;){
            std::unique_lock<std::mutex> lock(m1);

            while(head == nullptr){
                cv.wait(lock);
            }
            mp = head;
            head = mp->next;
            lock.unlock();

            std::cout << "consumer0:" <<mp->num << "\ttid" << std::this_thread::get_id() << std::endl;
            delete mp;
            sleep(rand()%3); 
        }
    });

        std::thread consumuer1([&head](){
        listNode* mp;
        for(;;){
            std::unique_lock<std::mutex> lock(m1);

            /* 手动检查条件，比较麻烦
            while(head == nullptr){
                cv.wait(lock);
            }
            */
            //更现代的写法
            cv.wait(lock,[&head](){ return head != nullptr;});
            mp = head;
            head = mp->next;
            lock.unlock();

            std::cout << "consumer1:" <<mp->num << "\ttid" << std::this_thread::get_id() << std::endl;
            delete mp;
            sleep(rand()%3); 
        }
    });

        std::thread consumuer2([&head](){
        listNode* mp;
        for(;;){
            std::unique_lock<std::mutex> lock(m1);

            while(head == nullptr){
                cv.wait(lock);
            }
            mp = head;
            head = mp->next;
            lock.unlock();

            std::cout << "consumer2:" <<mp->num << "\ttid" << std::this_thread::get_id() << std::endl;
            delete mp;
            sleep(3); 
        }
    });

    std::thread producer([&head = head](){
        for(;;){
            listNode* mp = new listNode;
            mp->num = rand()%900+1;
            std::cout << "producted-------" << mp->num << std::endl;
        
            m1.lock();
            mp->next = head;
            head = mp;
            m1.unlock();

            cv.notify_all();
            sleep(rand()%2);
        } 
    });

    consumuer.join();
    consumuer1.join();
    consumuer2.join();
    producer.join();
}