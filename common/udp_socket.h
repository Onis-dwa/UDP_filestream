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

// ������������ ������ ������
constexpr uint32_t packet_size = 1472;
constexpr uint32_t data_size = 1455;

enum class packetType: uint8_t {
	PUT = 0,
	ACK
};

struct packetData {
	uint32_t seq_number;     // ����� ������
	uint32_t seq_total;      // ���������� �������
	//uint8_t id[8];           // �������������
	uint64_t id;             // ������������� �� ��������, ��� �����
	packetType type;         // ��� ������ ���������� ����, ��-�� ������������
	uint8_t data[data_size]; // ������
};
static_assert(sizeof(packetData) == packet_size, "packet size incorrect");

struct metaData {
	int dataSize;      // ���������� ����������� ������
	uint32_t address;  // ����� �������
	uint16_t port;     // ���� �������
};

constexpr uint32_t POLY = 0x82f63b78;
static inline uint32_t crc32c(uint32_t crc, const unsigned char* buf, size_t len) {
	crc = ~crc;
	while (len--) {
		crc ^= *buf++;
		for (int k = 0; k < 8; k++)
			crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
	}
	return ~crc;
}

class udp_socket {
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

	int udp_send (const packetData& pd, const int dataSize); // �������� ��������� _address
	int udp_sendC(const packetData& pd, const metaData& md); // �������� �� �����������
	pair<metaData, packetData*> udp_recv();
	
	uint16_t getPort() const;
	uint32_t getAddr() const;

private:
#ifdef _WIN32
	SOCKET _socket = INVALID_SOCKET;
	sockaddr_in _address; // ���� ������ ��� �� ��������
	socklen_t _addressLen;
#elif __linux__
	int _socket;
	struct sockaddr_in _address;
#endif
	packetData _buffer = { 0 };
	
	status _status;
};
