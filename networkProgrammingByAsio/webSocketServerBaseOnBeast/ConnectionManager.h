#pragma once
#include "Connection.h"
#include <unordered_map>

class ConnectionMgr
{
public:
    static ConnectionMgr& GetInstance(); //实现单例模式
    void AddConnection(std::shared_ptr<Connection> conn_ptr); //连接建立后，将连接交给管理类管理
    void RmConnection(std::string uuid);    //用uuid来移除连接

private:
    ConnectionMgr(const ConnectionMgr&) = delete;
    ConnectionMgr& operator=(const ConnectionMgr&) = delete;
    ConnectionMgr();

    std::unordered_map<std::string,std::shared_ptr<Connection>> _map_conns; //用于存储复数连接和对应uuid
};