#pragma once
#include <iostream>
#include <memory>
#include <mutex>

template<class T>
class Singleton{
protected: //写protected，使得子类可以访问基类构造函数
    Singleton() = default;
    Singleton(const Singleton<T>&) = delete;
    Singleton& operator=(const Singleton<T>&) = delete;

    static std::shared_ptr<T> _instance; //用static保证该属性生命周期和程序生命周期一致

public:
    ~Singleton(){
        std::cout << "this Singleton instance destructed!" << std::endl;
    }

    static std::shared_ptr<T> GetInstance(){
        static std::once_flag s_flag;
        std::call_once(s_flag,[&](){  //确保只调用一次
            _instance = std::shared_ptr<T>(new T);
        });
        return _instance;
    }

    void GetAddress(){
        std::cout << _instance->get() << std::endl;
    }
};

template<typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;