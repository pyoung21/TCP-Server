#pragma once
#ifndef _SERVER_H_
#define _SERVER_H_

#include <WinSock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <Windows.h>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include "Client.h"

extern std::basic_ofstream<wchar_t> logger;

#define MAX_RECV_BUF MTU

class Server
{
private:

	HANDLE thCliRoutine;

	int socket;
	bool running;

	std::vector<Client> clients;
	sockaddr_in addr;

public:

	explicit Server(short port);
	~Server();

	void addClient(const Client client);
	void removeClient(int id);
	bool init(short port);
	void run();
	bool stop();
	bool executeCommand(std::vector<std::string>& args);
	inline const int& getSocket() const { return this->socket; }
	inline const bool& isRunning() const { return this->running; }
	inline const std::vector<Client>& getClients() const { return this->clients; }
};

#endif