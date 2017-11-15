#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <WinSock2.h>

#include "webserver.h"

WebServer::WebServer()
{
	threadH_ = nullptr;
}

void WebServer::run()
{
	WSADATA wsaData;
	int iResult;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed with error: %d\n", iResult);
		return;
	}
	std::string command;
	startListen();

	while (true)
	{
		std::cout << "\n>";
		getline(std::cin, command);
		if (command == "quit")
			break;
		else
			std::cout << "Invalid command";
	}

	std::cout << "Shutting down the server...\n";
	stopListen();
	WSACleanup();
	system("pause");
}

void WebServer::startListen()
{
	forTerminateThread_ = false;
	threadH_ = CreateThread(nullptr, 0, &listenThreadStatic, this, 0, &threadDW_);
}

void WebServer::stopListen()
{
	forTerminateThread_ = true;
	if (threadH_)
	{
		WaitForSingleObject(threadH_, INFINITE);
		CloseHandle(threadH_);
		threadH_ = nullptr;
	}
}

DWORD WINAPI WebServer::listenThreadStatic(void *param)
{
	WebServer *server = (WebServer*)param;
	server->listenThread();
	return 0;
}

#define SERVER_PORT "21345"

void WebServer::listenThread()
{
	struct addrinfo *result = nullptr;
	struct addrinfo hints;
	SOCKET listenSocket = INVALID_SOCKET;
	int iResult;
	fd_set readfds;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	//Resolve the server address and port
	iResult = getaddrinfo(nullptr, SERVER_PORT, &hints, &result);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		return;
	}

	// Create a SOCKET for connecting to server
	listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		return;
	}

	// Setup the TCP listening socket
	iResult = bind(listenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(listenSocket);
		return;
	}

	freeaddrinfo(result);

	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		return;
	}

	FD_ZERO(&readfds);
	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	while (!forTerminateThread_)
	{
		FD_SET(listenSocket, &readfds);
		int rv = select(1, &readfds, nullptr, nullptr, &tv);
		if (rv == -1)
		{
			perror("Select: some kind of an error");
		}
		else
			if (rv != 0) {
				if (FD_ISSET(listenSocket, &readfds)) {
					SOCKET clientSocket = accept(listenSocket, NULL, NULL);
					if (clientSocket != INVALID_SOCKET) {
						ConnectionHandler *connectionHandler = new ConnectionHandler(clientSocket);
						connectionHandler->start();
						clients_.push_back(connectionHandler);
						std::cout << "Connected a user." << std::endl;
					}
					else
						std::cout << "Some kind of a problem." << std::endl;
				}
			}
	}

	for (unsigned int i = 0; i < clients_.size(); ++i)
	{
		ConnectionHandler *ch = clients_[i];
		ch->stop();
		delete ch;
	}
	clients_.clear();

	// No longer need server socket
	closesocket(listenSocket);
}