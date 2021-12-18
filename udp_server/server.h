#pragma once

#include <string>
#include <map>
#include <chrono>
#include "udp_socket.h"

using namespace std;
using namespace std::chrono;

class server {
public:
	server();
	
	udp_socket::status start(const string& addr, const uint16_t port);
	
	uint16_t getServerPort() const;
	
	int exec();

private:
	struct packet {
		steady_clock::time_point lastActive;
		uint32_t seq_current;
		uint32_t seq_total;
		uint32_t len_total;
		uint32_t len_back;
		uint8_t* data; // [data_size * seq_total]
	};

	udp_socket _socket;
	map<uint64_t, packet> _packets;
	steady_clock::time_point lastExpiredCheck;
};