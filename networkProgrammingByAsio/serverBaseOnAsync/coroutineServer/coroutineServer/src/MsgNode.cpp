#include "../include/MsgNode.h"

RecvNode::RecvNode(int16_t msg_id,int16_t max_len) : _msg_id(msg_id),MsgNode(max_len){}

SendNode::SendNode(const char* msg,int16_t msg_id,int16_t max_len):
    _msg_id(msg_id),MsgNode(max_len + HEAD_TOTAL_LEN)
{
    //先发送id，转为网络字节序
    auto msg_id_net = boost::asio::detail::socket_ops::host_to_network_short(msg_id);
    memcpy(_data,&msg_id_net,HEAD_ID_LEN);
    //再写入字节长度，转为网络字节序
    auto max_len_net = boost::asio::detail::socket_ops::host_to_network_short(max_len);
    memcpy(_data + HEAD_ID_LEN,&max_len_net,HEAD_DATA_LEN);
    //最后写入要发送的数据
    memcpy(_data + HEAD_TOTAL_LEN,msg,max_len);
}