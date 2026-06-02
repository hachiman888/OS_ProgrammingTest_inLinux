//被wsl捅刀子，宿主机作为客户端发起请求永远会在升级协议那一步，立马断开连接，但是在wsl环境请求和申请就没有问题
//他妈的
#include "Connection.h"
#include "ConnectionManager.h"

Connection::Connection(net::io_context& ioc):
_ioc(ioc),_ws_ptr(std::make_unique<stream<tcp_stream>>(make_strand(ioc)))  //不可在初始化列表中直接传入_ioc，引用成员可能尚未初始化
//websocket本身并不线程安全，这样初始化由strand统一串行调度，保证线程安全
{
    //生成唯一uuid
    auto uuid = boost::uuids::random_generator()();
    _uuid = boost::uuids::to_string(uuid);
}

std::string Connection::GetUid(){
    return _uuid;
}

net::ip::tcp::socket& Connection::GetSocket(){
    //最佳实践
    return boost::beast::get_lowest_layer(*_ws_ptr).socket(); 
    //返回websocket最底层的socket的引用
}

void Connection::AsyncAccept(){
    auto self = shared_from_this();     //延长Connection对象生命周期

    ConnectionMgr::GetInstance().AddConnection(self);  //连接建立成功，将连接交由管理类管理，提前加入哈希表中，保证connection不在strand空档中提前析构
    _ws_ptr->async_accept([self](boost::system::error_code ec){ 
    // websocket异步接收连接。调用这个函数意味着，在tcp基础上，将协议升级为websocket
        try{
            if(!ec){            
                self->Start();  //若连接建立成功，则开始进入读写逻辑 
            }else{
                std::cerr << "websocket async accept error!" 
                    << "\terror is " << ec.what() << std::endl;
                ConnectionMgr::GetInstance().RmConnection(self->_uuid); //升级失败，移除连接
            }
        }catch(std::exception& e){
            std::cerr << "websocket async accept exception is " 
                << e.what() << std::endl;
        }
    });            
}

void Connection::AsyncSend(std::string msg){
    {
        std::lock_guard<std::mutex> lock_guard(_send_mtx);
        auto que_len = _send_queue.size();
        _send_queue.push(msg);
        if(que_len > 0)
            return;
    }

    SendCallBack(std::move(msg));
}

void Connection::SendCallBack(std::string msg){
    auto self = shared_from_this();
    _ws_ptr->async_write(boost::asio::buffer(msg.c_str(),msg.size()),
    [self](error_code ec,std::size_t buffer_bytes)
    {
        try{    //对端异常关闭，发送会触发异常，所以得写try和catch
            if(ec){
                std::cout << "async send error is " << ec.what() << std::endl;
                ConnectionMgr::GetInstance().RmConnection(self->_uuid);
                return;
            }

            std::string send_msg;
            {
                std::lock_guard<std::mutex> lk_guard(self->_send_mtx);
                self->_send_queue.pop();
                if(self->_send_queue.empty()) //若队列为空，则表示处理完成，直接退出
                    return;

                //若队列不为空，则取出剩余数据
                send_msg = self->_send_queue.front();
            }
            self->SendCallBack(std::move(send_msg)); //将剩余数据发送
        }catch(std::exception& e){
            std::cout << "async send exception is " << e.what() << std::endl;
            ConnectionMgr::GetInstance().RmConnection(self->_uuid);
        }
    });
}

void Connection::Start(){
    auto self = shared_from_this();
    _ws_ptr->async_read(_recv_buffer,[self]
        (boost::system::error_code ec,std::size_t buff_bytes)
        {
            try{
                if(ec){
                    std::cerr << "websocket async read error is "
                    << ec.what() << std::endl;
                    ConnectionMgr::GetInstance().RmConnection(self->GetUid());
                    return ;
                }
                self->_ws_ptr->text(self->_ws_ptr->got_text()); //判断接收到的消息是否是文本类型
                std::string recv_data = beast::buffers_to_string(self->_recv_buffer.data()); //将缓冲区的数据转换成string
                self->_recv_buffer.consume(self->_recv_buffer.size());  //清空缓存区，方便复用
                std::cout << "websocket receive msg is " << recv_data << std::endl;

                self->AsyncSend(std::move(recv_data));  //接收并处理数据后，回发数据
                self->Start(); //一轮处理结束，继续监听事件
            }catch(std::exception& e){
                std::cerr << "websocket async read exception is "
                    << e.what() << std::endl;
                ConnectionMgr::GetInstance().RmConnection(self->GetUid());
            }
        }); //注册异步监听读写事件
}