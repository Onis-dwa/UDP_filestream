#pragma once

#include <cstdint>

#ifdef _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
int initWSSock(); // init windows sockets

#elif  __linux__
#include <sys/socket.h>

#endif

// максимальный размер буфера
constexpr uint32_t data_size = 1455;

class udp_socket {
public:
    enum class status : uint8_t {
        no_init = 0,
    	err_init,
    	err_addr,
    	err_bind,

    	bind,
    	closed,
    };
	
    udp_socket();

    //accept();
    status sbind(const char* address, uint16_t port);
	
    //send();
    //recv();

    uint16_t getPort();

private:
#ifdef _WIN32
    SOCKET _socket = INVALID_SOCKET;
#elif __linux__
    int _socket;
    struct sockaddr_in _address;
#endif
    char _buffer[data_size] = { 0 };

    uint16_t port;
	// string addr?
};
