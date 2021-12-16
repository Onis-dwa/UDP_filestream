#include <iostream>
#include "udp_socket.h"

using std::cout;
using std::endl;

int initWSSock() {
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
	//_socket(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)), // tcp
	_socket(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)),
	_buffer{0}
{}

udp_socket::status udp_socket::sbind(const char* address, uint16_t port) {
	// проверка корректности инициализации
	if (_socket == INVALID_SOCKET)
		return status::no_init;

	// Структура хост/порт/протокол
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	//sin.sin_addr.s_addr = inet_addr(address); deprecated
	if (inet_pton(AF_INET, address, &sin.sin_addr) != 1)
		return status::err_addr;
	sin.sin_port = htons(port);

	// связываем сокет с адресом и портом
	if (bind(_socket, (SOCKADDR*)&sin, sizeof(sin)) == SOCKET_ERROR)
		return status::err_bind;
	// если tcp то нужно ещё вызвать listen
	return status::bind;
}

uint16_t udp_socket::getPort() {
	sockaddr_in sin;
	socklen_t len = sizeof(sin);
	if (getsockname(_socket, (SOCKADDR*)&sin, &len) == -1)
		return 0;
	else
		return ntohs(sin.sin_port);
}