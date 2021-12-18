
#include <iostream>
#include "client.h"

using namespace std;

int main() {
#ifdef _WIN32
	if (initWSSock()) { return 1; } // init failed
#endif
	client cl;
	auto rc = cl.connectTo("127.0.0.1", 32094);
	if (rc != udp_socket::status::init) {
		cout << "Err on init. status: " << (uint32_t)rc
			<< " WinError: " << WSAGetLastError() << endl;
		return 1;
	}
	cout << "Connected?" << endl;

	srand(time(0));
	const auto erc = cl.exec();
	cout << "Server stopped with code: " << erc << endl;
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}
