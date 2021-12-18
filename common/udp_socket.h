#pragma once

#include <string>
#include <cstdint>

#ifdef _WIN32
#include <winsock2.h>
#include <WS2tcpip.h>

#pragma comment(lib, "ws2_32.lib")
int initWSSock(); // init windows sockets

#elif  __linux__
#include <sys/socket.h>

#endif

using std::string;
using std::pair;

// максимальный размер буфера
constexpr uint32_t packet_size = 1472;
constexpr uint32_t data_size = 1455;

// энумы и структуры пакета
enum class packetType: uint8_t {
	PUT = 0,
	ACK
};
struct packetData {
	uint32_t seq_number;     // номер пакета
	uint32_t seq_total;      // количество пакетов
	//uint8_t id[8];           // идентификатор
	uint64_t id;             // идентификатор не массивом, так проще
	packetType type;         // тип пакета переместил вниз, из-за выравнивания
	uint8_t data[data_size]; // данные
};
static_assert(sizeof(packetData) == packet_size, "packet size incorrect");
struct metaData {
	int dataSize;      // количество прочитанных данных
	uint32_t address;  // адрес клиента
	uint16_t port;     // порт клиента
};

class udp_socket {
	/* кросс платформенная обёртка для сокета */
public:
	enum class status : uint8_t {
		no_init = 0,
		err_init,
		err_addr,
		err_bind,

		init,
		bind,

		init_req,
		stop_req,
	};
	
	udp_socket();

	status init(const char* address, uint16_t port);
	status start();
	status stop();

	int udp_send (const packetData& pd, const int dataSize); // отправка используя _address
	int udp_sendC(const packetData& pd, const metaData& md); // отправка по креденшелам
	pair<metaData, packetData*> udp_recv();
	
	uint16_t getPort() const;
	uint32_t getAddr() const;

private:
#ifdef _WIN32
	SOCKET _socket = INVALID_SOCKET;
	sockaddr_in _address; // дабы каждый раз не генерить
	socklen_t _addressLen;
#elif __linux__
	int _socket;
	struct sockaddr_in _address;
#endif
	packetData _buffer = { 0 };
	
	status _status;
};
