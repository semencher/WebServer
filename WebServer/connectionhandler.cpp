#include <string>
#include <iostream>
#include <fstream>
#include <vector>

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
	std::vector<std::string> names;
	names.push_back("index.html");
	names.push_back("favicon.ico");
	names.push_back("leo.jpg");
	names.push_back("leo.mp4");
	names.push_back("s.jpg");

	fd_set readfds;
	FD_ZERO(&readfds);
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	
	char buffer[IN_BUFFER_SIZE + 1];
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
					int result = 0;
					std::string nameFile;
					int read = recv(socket_, buffer, IN_BUFFER_SIZE, 0);
					if (read > 0) {
						std::string str(buffer);
						// TODO: Убрать эту строчку когда закончу.
						int i = 5;
						while (true) {
							if (str[i] == ' ') {
								break;
							}
							nameFile += str[i];
							++i;
						}
						if (nameFile == "") {
							nameFile = "index.html";
						}
						else {
							if (std::find(names.begin(), names.end(), nameFile) == names.end()) {
								std::string answer = "HTTP/1.1 403\r\n";
								answer += "Server: localhost:21345\r\n";
								answer += "Content - Language : ru\r\n";
								answer += "Connection : close \r\n\r\n";

								result = send(socket_, answer.c_str(), answer.size(), 0);
								if (result == SOCKET_ERROR)
								{
									printf("send from client failed with error: %d\n", WSAGetLastError());
									break;
								}
								forTerminateThread_ = true;
								continue;
							}
						}
						std::cout << "\n" + nameFile + "\n";

						std::string page;
						std::fstream file;
						file.open(nameFile, std::ios::in | std::ios::binary);

						file.seekg(0, std::ios::end);
						size_t length = static_cast<size_t>(file.tellg());
						file.seekg(0, std::ios::beg);

						std::string answer = "HTTP/1.1 200 OK\r\n";
						answer += "Server: localhost:21345\r\n";
						answer += "Content - Language : ru\r\n";
						answer += "Content - Type : text / html; charset = utf - 8\r\n";
						answer += "Content - Length: " + std::to_string(length) + "\r\n";
						answer += "Connection : close \r\n\r\n";

						result = send(socket_, answer.c_str(), answer.size(), 0);
						if (result == SOCKET_ERROR)
						{
							printf("send from client failed with error: %d\n", WSAGetLastError());
							break;
						}

						int sent = 0;
						while (length > IN_BUFFER_SIZE) {
							file.read(buffer, IN_BUFFER_SIZE);
							buffer[IN_BUFFER_SIZE] = '\0';
							result = send(socket_, buffer, IN_BUFFER_SIZE, 0);
							if (result == SOCKET_ERROR)
							{
								printf("send from client failed with error: %d\n", WSAGetLastError());
								break;
							}
							sent += result;
							length = length - IN_BUFFER_SIZE;
						}
						if (length > 0) {
							file.read(buffer, length);
							buffer[length] = '\0';
							result = send(socket_, buffer, length, 0);
							if (result == SOCKET_ERROR)
							{
								printf("send from client failed with error: %d\n", WSAGetLastError());
								break;
							}
							sent += result;
						}
						forTerminateThread_ = true;
					}
				}
			}
	}
	shutdown(socket_, 2);
	closesocket(socket_);
}