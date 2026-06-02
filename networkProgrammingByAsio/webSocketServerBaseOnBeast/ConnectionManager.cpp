#include "ConnectionManager.h"

ConnectionMgr::ConnectionMgr(){}

ConnectionMgr& ConnectionMgr::GetInstance(){
    static ConnectionMgr instance;
    return instance;
}

void ConnectionMgr::AddConnection(std::shared_ptr<Connection> conn_ptr){
    _map_conns[conn_ptr->GetUid()] = conn_ptr;
}

void ConnectionMgr::RmConnection(std::string uuid){
    _map_conns.erase(uuid);
}