#include <iostream>
#include <string>
#include <boost/asio.hpp>
namespace asio = boost::asio;
namespace ip = asio::ip;
using tcp = ip::tcp;

int main() {
	std::string target_ip_address = "127.0.0.1";
	unsigned short port_num = 8888;

	tcp::endpoint ep(ip::make_address(target_ip_address), port_num);
	asio::io_context io;
	boost::system::error_code ec;
	char buf[BUFSIZ];
	std::size_t n = 0;
	std::string msg = "hello world!";

	tcp::socket sock(io, ep.protocol());
	sock.connect(ep, ec);
	if (ec) {
		std::cerr << "connect error!" << ec.message() <<std::endl;
		return -1;
	}

	for (int i = 0; i < 5; i++) {
		asio::write(sock, asio::buffer(msg), ec);
		if (ec) {
			std::cerr << "send error!" << ec.message() <<std::endl;
			break;
		}

		n = sock.receive(asio::buffer(buf, BUFSIZ), 0, ec);
		if (ec) {
			std::cerr << "receive error!" << ec.message() <<std::endl;
			break;
		}

		std::cout << "Echo from Server:" << std::string(buf, n) << std::endl;
	}
}