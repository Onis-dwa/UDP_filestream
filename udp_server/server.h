#pragma once

#include <string>
#include "udp_socket.h"

using std::string;

class server {
public:
	server();
	
	udp_socket::status start(const string& addr, const uint16_t port);
	
	uint16_t getServerPort() const;
	
	int exec();

private:

	udp_socket _socket;
};