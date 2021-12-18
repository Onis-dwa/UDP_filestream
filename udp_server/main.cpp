﻿
#include <iostream>
#include "server.h"

using namespace std;

int main() {
#ifdef _WIN32
	if (initWSSock()) { return 1; } // init failed
#endif
	
	server srv;
	auto rc = srv.start("127.0.0.1", 32094);
	if (rc != udp_socket::status::bind) {
		cout << "Err on bind. status: " << (uint32_t)rc
			<< " WinError: " << WSAGetLastError() << endl;
		return 1;
	}
	cout << "Bind success on port: " << srv.getServerPort() << endl;
	
	const auto erc = srv.exec();
	
	cout << "Server stopped with code: " << erc << endl;
#ifdef _WIN32
	WSACleanup();
#endif
	cin.get();
	return 0;
}
