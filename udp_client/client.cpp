#include <iostream>
#include "client.h"

using std::cout;
using std::endl;

client::client():
	_socket()
{}

udp_socket::status client::connectTo(const string& addr, const uint16_t port) {
	return _socket.init(addr.c_str(), port);
}

int client::exec() {
	metaData md;
	//md.address = _socket.getAddr();
	//md.port = _socket.getPort();
	md.address = 0;
	md.port = 0;

	packetData pd {
		123,
		123,
		0,
		packetType::PUT,
		{'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A', 'A'}
	};
	while (true) {
		md.dataSize = rand() % data_size;
		for (int i = 0; i < md.dataSize; ++i) {
			pd.data[i] = std::rand() % 94 + 33;
		}
		auto rc = _socket.udp_send(pd, md.dataSize);
		if (rc == SOCKET_ERROR)
			cout << "err send: " << WSAGetLastError() << endl;
		else
			cout << "send: " << rc << endl;
	}
	
	return 0;
}
