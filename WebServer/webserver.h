#pragma once

#include <vector>

#include "connectionhandler.h"

class WebServer
{
public:
	WebServer();
	void run();

private:
	void startListen();
	void stopListen();
	static DWORD WINAPI listenThreadStatic(void *param);
	void listenThread();

private:
	bool forTerminateThread_;
	HANDLE threadH_;
	DWORD threadDW_;

	std::vector<ConnectionHandler *> clients_;

};	// class WebServer