#include "webserver.h"

#pragma comment (lib, "Ws2_32.lib")

int main()
{
	WebServer server;
	server.run();
	return 0;
}