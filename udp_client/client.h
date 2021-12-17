#pragma once

#include <string>
#include "udp_socket.h"

using std::string;

class client {
public:
	client();

	udp_socket::status connectTo(const string& addr, const uint16_t port);

	int exec();

private:

	udp_socket _socket;
};