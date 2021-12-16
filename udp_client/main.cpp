
#include <iostream>
#include "client.h"

using namespace std;

int main() {
#ifdef _WIN32
	if (initWSSock()) { return 1; } // init failed
#endif
	client srv;
	srv.connect();

	cout << "end" << endl;
#ifdef _WIN32
	WSACleanup();
#endif
	return 0;
}
