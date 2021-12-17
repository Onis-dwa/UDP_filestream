#include <iostream>
#include "udp_socket.h"

using std::cout;
using std::endl;
using std::move;

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
	_address { 0 },
	_addressLen(0),
	_buffer{0},
	_status(status::no_init)
{}

udp_socket::status udp_socket::init(const char* address, uint16_t port) {
	if (_status != status::no_init)
		return status::stop_req;
	
	// инит unbound сокета
	//                        SOCK_STREAM, IPPROTO_TCP
	//                        SOCK_DGRAM,  IPPROTO_UDP
	_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	// Структура хост/порт/протокол
	_address.sin_family = AF_INET;
	//_address.sin_addr.s_addr = inet_addr(address); deprecated
	if (inet_pton(AF_INET, address, &_address.sin_addr) != 1)
		return _status = status::err_addr;
	_address.sin_port = htons(port);
	_addressLen = sizeof(_address);

	return _status = status::init;
}
udp_socket::status udp_socket::start() {
	// старт только при no_init
	if (_status != status::init)
		return status::init_req;
	
	// связываем сокет с адресом и портом
	if (bind(_socket, (SOCKADDR*)&_address, _addressLen) == SOCKET_ERROR)
		return _status = status::err_bind;
	
	// when tcp
	//if (listen(_socket, SOMAXCONN) == SOCKET_ERROR)
	//	return _status = status::err_listen;

	// получаем реальный порт
	if (_address.sin_port == 0) {
		if (getsockname(_socket, (SOCKADDR*)&_address, &_addressLen) == -1) {
			_address = { 0 };
			_address.sin_port = 0;
		}
	}
	
	return _status = status::bind;
}
udp_socket::status udp_socket::stop() {
	closesocket(_socket);
	return _status = status::no_init;
}

uint16_t udp_socket::getPort() const {
	return ntohs(_address.sin_port);
}
uint32_t udp_socket::getAddr() const {
	return _address.sin_addr.S_un.S_addr;
}

int udp_socket::udp_send(const packetData& pd, const int dataSize) {
	return sendto(_socket, (char*)&pd, dataSize, 0, (SOCKADDR*)&_address, _addressLen);
}
int udp_socket::udp_sendC(const packetData& pd, const metaData & md) {
	// заполняем структуру клиента
	sockaddr_in client;
	client.sin_family = AF_INET;
	client.sin_addr.S_un.S_addr = md.address;
	client.sin_port = htons(md.port);
	return sendto(_socket, (char*)&pd, md.dataSize, 0, (SOCKADDR*)&client, sizeof(client));
}
pair<metaData, packetData*> udp_socket::udp_recv() {
	// получение данных
	sockaddr_in client;
	int clientSize = sizeof(client);
	auto rc = recvfrom(_socket, (char*)&_buffer, packet_size, 0, (SOCKADDR*)&client, &clientSize);

	// заполнение мета данных
	metaData md;
	md.address = client.sin_addr.S_un.S_addr;
	md.port = client.sin_port;
	md.dataSize = rc;

	// проверка нужно ли передовать буфер
	if (rc > 0) {
		return { move(md), &_buffer };
	}
	if (rc == 0){
		return { move(md), nullptr};
	}
	return { move(md), nullptr};
}
