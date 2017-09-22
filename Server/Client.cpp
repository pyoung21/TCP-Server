#include "Client.h"

extern std::mutex gClientMutex;

Client::~Client()
{
}

Client::Client(int id, int socket, sockaddr_in addr)
{
	this->connected = true;
	this->trusted = false;
	this->id = id;
	this->socket = socket;
	this->addr = addr;
}

void Client::enqueuePacket(const _packet& packet)
{
	this->incomingPackets.push_back(packet);
}