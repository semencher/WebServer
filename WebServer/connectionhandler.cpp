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
					int result = 0;
					int sizeBuffer = 400;
					std::string nameFile;
					int read = recv(socket_, buffer, IN_BUFFER_SIZE, 0);
					if (read > 0) {
						std::string str(buffer);
						// TODO: Убрать эту строчку когда закончу.
						std::cout << "\n" << str << "\n";
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
							// Проверить на неккоректное имя, и ответить сайту что мы это не дадим.
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
						while (length > sizeBuffer) {
							file.read(buffer, sizeBuffer);
							buffer[sizeBuffer] = '\0';
							result = send(socket_, buffer, sizeBuffer, 0);
							if (result == SOCKET_ERROR)
							{
								printf("send from client failed with error: %d\n", WSAGetLastError());
								break;
							}
							sent += result;
							length = length - sizeBuffer;
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

// Сложный пример.
// Реакция на запрещенный ресурс.
// Тип контента менять для разных типов.