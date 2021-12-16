#pragma once

#include "udp_socket.h"

class server {
public:
	server();
	int exec();
private:

	udp_socket _socket;
};