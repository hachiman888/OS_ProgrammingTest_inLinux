#pragma once
#include <iostream>
#include <string>
#include <boost/asio.hpp>

class Session;

class Msg_Node{
public:
    Msg_Node(short max_len):_total_len(max_len),_cur_len(0){ //变量一定要初始化，于此处被狠狠折磨3个小时
        _data = new char[_total_len + 1]();
        _data[_total_len] = '\0';
    }

    ~Msg_Node(){
        std::cout << "MsgNode destructed" << std::endl; 
        delete[] _data;
        _cur_len = 0;
        _total_len = 0;
    }

    void Clear(){
        ::memset(_data,0,_total_len);
        _cur_len = 0;
    }

    short _cur_len;
    short _total_len;
    char* _data;
};

class Recv_Node : public Msg_Node{
    friend class Session;
public:
    Recv_Node(short max_len,short msg_id);

private:
    short _msg_id;
};

class Send_Node : public Msg_Node{
    friend class Session;
public:
    Send_Node(const char* msg,short max_len,short msg_id);

private:
    short _msg_id;
};