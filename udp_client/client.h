#pragma once

#include <vector>
#include <string>
#include <chrono>
#include "udp_socket.h"

using namespace std;
using namespace std::chrono;

class client {
public:
	client();

	udp_socket::status connectTo(const string& addr, const uint16_t port);

	int exec();
	
private:
	uint32_t getNext();
	bool getId();
	bool sendAll();
	bool checkCheckSumm(const uint8_t* data, const int size);
	
	udp_socket _socket;
	// отправляемая дата
	vector<packetType> _packetsInfo;
	uint32_t _packsCount;
	uint8_t* _rawData;
	uint32_t _totalLen;
	uint32_t _backLen;
	
	packetData _pd;
	uint32_t _packsLeft;
	bool _checksummCorrect;
};