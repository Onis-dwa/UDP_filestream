#pragma once

#include "udp_socket.h"

class client {
public:
	client();
	int connect();
private:

	udp_socket _socket;
};