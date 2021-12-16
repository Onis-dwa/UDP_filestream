#include <iostream>
#include "udp_socket.h"

using std::cout;
using std::endl;

int initWSSock() {
	// from https://docs.microsoft.com/en-us/windows/win32/api/winsock2/nf-winsock2-wsastartup
	static WSADATA wsaData;
	if (const int rc = WSAStartup(MAKEWORD(2, 2), &wsaData)) {
		cout << "WSAStartup failed with error: " << rc << endl;
		return 1;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
		cout << "Could not find a usable version of Winsock.dll\n";
		WSACleanup();
		return 1;
	}
	return 0;
}

udp_socket::udp_socket():
	_socket(INVALID_SOCKET),
	_buffer{0},
	_status(status::no_init),
	_port(0)
{}

udp_socket::status udp_socket::start(const char* address, uint16_t port) {
	// старт только при no_init
	if (_status != status::no_init)
		return status::stop_req;

	// инит unbound сокета
	//                        SOCK_STREAM, IPPROTO_TCP
	//                        SOCK_DGRAM,  IPPROTO_UDP
	_socket = socket(AF_INET, SOCK_DGRAM,  IPPROTO_UDP);
	
	// проверка корректности инициализации
	if (_socket == INVALID_SOCKET)
		return _status = status::no_init;

	// Структура хост/порт/протокол
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	//sin.sin_addr.s_addr = inet_addr(address); deprecated
	if (inet_pton(AF_INET, address, &sin.sin_addr) != 1)
		return _status = status::err_addr;
	sin.sin_port = htons(port);
	socklen_t len = sizeof(sin);

	// связываем сокет с адресом и портом
	if (bind(_socket, (SOCKADDR*)&sin, len) == SOCKET_ERROR)
		return _status = status::err_bind;
	
	// when tcp
	//if (listen(_socket, SOMAXCONN) == SOCKET_ERROR)
	//	return _status = status::err_listen;

	// получаем реальный порт
	if (port == 0) {
		if (getsockname(_socket, (SOCKADDR*)&sin, &len) == -1)
			_port = 0;
		_port = ntohs(sin.sin_port);
	}
	else
		_port = port;
	
	return _status = status::bind;
}
udp_socket::status udp_socket::stop() {
	closesocket(_socket);
	return _status = status::no_init;
}

uint16_t udp_socket::getPort() const {
	return _port;
}
