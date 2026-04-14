#include <iostream>
#include <boost/asio.hpp>
#include <string>

int main() {
	unsigned short port_num = 8888;
	boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address_v4::any(), port_num);
	boost::asio::io_context io;
	const int BACKLOG = 30;
	boost::system::error_code ec;

	boost::asio::ip::tcp::acceptor a(io, ep.protocol());
	a.bind(ep);
	a.listen(BACKLOG);
	boost::asio::ip::tcp::socket sock(io);
	a.accept(sock); //于此阻塞等待
	char buf[BUFSIZ];
	std::size_t n = 0;

	while (1) {
        std::cout << "server is waiting...";
		n = sock.receive(boost::asio::buffer(buf, BUFSIZ),0,ec); //阻塞等待对端发送信息
		if (ec) {
			if (ec == boost::asio::error::eof) {
				std::cout << "connection closed" << std::endl;
			}
			else {
				std::cout << "Error: " << ec.message() << std::endl;
			}
			break;
		}

		std::cout << "msg is:" << std::string(buf, n) << std::endl;
		
		n = sock.send(boost::asio::buffer(buf, n),0,ec);
		if (ec) {
			std::cout << "send failed" << std::endl;
			break;
		}
	}
}