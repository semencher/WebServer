#include <string>
#include <iostream>
#include <fstream>

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
						// TODO: Убрать эту строчку когда закончу.
						std::cout << "\n" << str << "\n";

						std::string page;
						std::fstream file;
						file.open("index.html", std::ios::in | std::ios::binary);

						file.seekg(0, std::ios::end);
						size_t length = static_cast<size_t>(file.tellg());
						file.seekg(0, std::ios::beg);

						while (length > IN_BUFFER_SIZE) {
							file.read(buffer, IN_BUFFER_SIZE);
							page += std::string(buffer);
							length -= IN_BUFFER_SIZE;
						}
						if (length > 0) {
							file.read(buffer, length);
							buffer[length] = '\0';
							page += std::string(buffer);
						}

						std::string answer = "HTTP/1.1 200 OK\n";
						answer += "Server: localhost:21345\n";
						answer += "Content - Language : ru\n";
						answer += "Content - Type : text / html; charset = utf - 8\n";
						answer += "Content - Length: " + std::to_string(page.size()) + "\n";
						answer += "Connection : close \n\n";
						answer += page;

						// Отправляем ответ.
						int result = send(socket_, answer.c_str(), answer.size(), 0);
						if (result == SOCKET_ERROR)
						{
							printf("send from client failed with error: %d\n", WSAGetLastError());
							break;
						}
					}
				}
			}
	}
}