#include <iostream>
#include "server.h"

using std::cout;
using std::endl;

server::server():
	_socket()
{}

udp_socket::status server::start(const string & addr, const uint16_t port) {
	if (_socket.init(addr.c_str(), port) != udp_socket::status::init)
		return udp_socket::status::init;
	return _socket.start();
}

uint16_t server::getServerPort() const {
	return _socket.getPort();
}

int server::exec() {
	while (true) {
		auto p = _socket.udp_recv();
		cout << "recv: " << p.first.dataSize << endl;
	}
	return 0;
}
