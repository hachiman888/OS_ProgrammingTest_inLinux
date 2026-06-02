#include "MsgNode.h"
#include "const.h"

Recv_Node::Recv_Node(short max_len,short msg_id):Msg_Node(max_len),_msg_id(msg_id){}

Send_Node::Send_Node(const char* msg,short max_len,short msg_id)
    :Msg_Node(max_len + HEAD_TOTAL_LEN),_msg_id(msg_id)
{
    //先发id，将其转换为网络字节序
    short msg_id_net = boost::asio::detail::socket_ops::host_to_network_short(_msg_id);
    memcpy(_data,&msg_id_net,HEAD_ID_LEN);
    short max_len_net = boost::asio::detail::socket_ops::host_to_network_short(max_len);
    memcpy(_data + HEAD_ID_LEN,&max_len_net,HEAD_DATA_LEN);
    memcpy(_data + HEAD_TOTAL_LEN,msg,max_len);
}