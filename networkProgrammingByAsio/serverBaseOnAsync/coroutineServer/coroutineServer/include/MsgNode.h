#pragma once
#include "../include/const.h"
#include <string>
#include <iostream>
#include <boost/asio.hpp>
#include "../include/LogicSystem.h"

class LogicSystem;
class MsgNode {
public:
    MsgNode(unsigned short max_len) : _total_len(max_len),_cur_len(0){
        _data = new char[max_len + 1]();
        _data[_total_len] = '\0';
    }

    ~MsgNode(){
        delete[] _data;
        std::cout << "MsgNode destructed..." << std::endl;
    }

    void Clear(){
        memset(_data,0,_total_len);
        _cur_len = 0;
    }

    char* _data;
    short _cur_len;
    short _total_len;
};

class RecvNode : public MsgNode {
    friend class LogicSystem;
public:
    RecvNode(int16_t msg_id,int16_t max_len);
private:
    int16_t _msg_id;
};

class SendNode : public MsgNode {
public:
    SendNode(const char* msg,int16_t msg_id,int16_t max_len);
private:
    int16_t _msg_id;
};