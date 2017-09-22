#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <Ws2tcpip.h>
#include <Windows.h>
#include <mutex>
#include <iostream>
#include <fstream>
#include <queue>
#include "Packet.h"

extern std::mutex gClientMutex;

class Client
{
public:

	int id;
	int socket;
	bool connected;
	bool trusted;
	std::string name;
	sockaddr_in addr;
	std::vector<_packet> incomingPackets;

public:

	friend class Server;

	friend std::ostream& operator<<(std::ostream& os, const Client& c)
	{
		os
			<< c.id << " "
			<< c.getIp() << ":" << c.getPort()
			<< " Trusted: "
			<< (c.trusted == true ? "True" : "False");

		return os;
	}

	explicit Client(int id, int socket, sockaddr_in addr);
	~Client();

	const int&			getSocket() const { return this->socket; }
	const int&			getId() const { return this->id; }
	const sockaddr_in&	getAddr() const { return this->addr; }
	const uint16_t		getPort() const { return this->addr.sin_port; }
	const std::string&	getName() const { return this->name; }
	const bool&			isTrusted() const { return this->trusted; }
	void				enqueuePacket(const _packet& packet);
	const std::string	getIp() const 
	{ 
		char addr[32] = { 0 };
		inet_ntop(AF_INET, &this->addr.sin_addr.s_addr, &addr[0], sizeof(addr));
		return (addr);
	}
};

#endif