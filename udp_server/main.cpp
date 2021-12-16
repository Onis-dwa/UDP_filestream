
#include <iostream>
#include "server.h"

using namespace std;

int main() {
#ifdef _WIN32
	if (initWSSock()) { return 1; } // init failed
#endif
	server srv;
	srv.exec();
	
	cout << "end" << endl;
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}
