#pragma once

#include <Windows.h>

class ConnectionHandler
{
public:
	ConnectionHandler(SOCKET socket);
	void start();
	void stop();

private:
	static DWORD WINAPI handlerThreadStatic(void *param);
	void handlerThread();

private:
	SOCKET socket_;
	bool forTerminateThread_;
	HANDLE threadH_;
	DWORD threadDW_;

};	// class ConnectionHandler