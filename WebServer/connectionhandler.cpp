#include <string>
#include <iostream>

#include "connectionhandler.h"

ConnectionHandler::ConnectionHandler(SOCKET socket)
{
	this->socket_ = socket;
	threadH_ = nullptr;
}

void ConnectionHandler::start()
{
	forTerminateThread_ = false;
	threadH_ = CreateThread(nullptr, 0, &handlerThreadStatic, this, 0, &threadDW_);
}

void ConnectionHandler::stop()
{
	forTerminateThread_ = true;
	if (threadH_)
	{
		WaitForSingleObject(threadH_, INFINITE);
		CloseHandle(threadH_);
		threadH_ = nullptr;
	}
}

DWORD WINAPI ConnectionHandler::handlerThreadStatic(void *param)
{
	ConnectionHandler *connectionHandler = (ConnectionHandler*)param;
	connectionHandler->handlerThread();
	return 0;
}

#define IN_BUFFER_SIZE	1024

void ConnectionHandler::handlerThread()
{
	fd_set readfds;
	FD_ZERO(&readfds);
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;



	char buffer[IN_BUFFER_SIZE];
	while (!forTerminateThread_)
	{

		FD_SET(socket_, &readfds);
		int rv = select(1, &readfds, NULL, NULL, &tv);
		if (rv == -1)
		{
			perror("Select: "); // error occurred in select()
		}
		else
			if (rv != 0) {
				if (FD_ISSET(socket_, &readfds))
				{
					int read = recv(socket_, buffer, IN_BUFFER_SIZE, 0);
					if (read > 0) {
						std::string str(buffer);
						std::cout << "\n" << str << "\n";
					}
				}
			}
	}
}