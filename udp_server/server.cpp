#include <iostream>
#include "server.h"

using std::cout;
using std::endl;

server::server() {
}

int server::exec() {
	auto rc = _socket.start("127.0.0.1", 32094);
	if (rc != udp_socket::status::bind) {
		cout << "err on bind. status: " << (uint32_t)rc << " " << WSAGetLastError() << endl;
		return 1;
	}
	
	cout << "listen success. port: " << _socket.getPort() << endl;;
	return 0;
}
