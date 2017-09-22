#ifndef _TCPCONNECTION_H_
#define _TCPCONNECTION_H_

#include <WS2tcpip.h>
#pragma comment(lib,"Ws2_32.lib")
#include <cstring>
#include <iostream>
#include "Packet.h"

#define MAX_BUFF_SIZE 256
typedef unsigned char byte;

class TCPConnection
{
	byte		receiveBuffer[MAX_BUFF_SIZE + 1];
	byte		sendBuffer[MAX_BUFF_SIZE + 1];
	int			buffSize;
	int			socket;
	std::string ipAddress;
	size_t		port;

public:

	friend class TCPConnector;

	~TCPConnection()
	{
		closesocket(socket);
		WSACleanup();
		free(this);
	}

	void close()
	{
		closesocket(socket);
		WSACleanup();
		free(this);
	}

	int sendData(const byte *data, size_t size)
	{
		if (data)
		{
			int ret = send(socket, (const char*)data, size, 0);
			return ret;
		}
		return 0;
	}

	TCPConnection& operator<<(std::string& data)
	{
		send(socket, data.c_str(), data.size(), 0);
		return *this;
	}

	TCPConnection& operator<<(_packet& packet)
	{
		std::vector<byte> buffer(packet.header.size, 0);
		writePacket(buffer.data(), buffer.size(), &packet);
		send(socket, (const char*)buffer.data(), buffer.size(), 0);
		return *this;
	}

	TCPConnection& operator<<(const char *data)
	{
		if (data)
		{
			size_t ret = send(socket, data, strlen(data), 0);
		}
		return *this;
	}

	unsigned int sendBytes(const char *data, size_t size)
	{
		if (data && size > 0)
		{
			size_t ret = send(socket, data, size, 0);
			return ret;
		}
		return 0;
	}

	SSIZE_T rcv_data()
	{
		memset(receiveBuffer, 0, buffSize);
		size_t ret = recv(socket, (char*)receiveBuffer, buffSize, 0);

		return ret;
	}

	inline const byte* rcv_buffer()	const { return this->receiveBuffer; }
	inline const std::string& getIp() const { return this->ipAddress; }
	inline const size_t& getPort() const { return this->port; }
	inline const SOCKET& getSocket() const { return this->socket; }

private:

	TCPConnection(SOCKET sock, SOCKADDR_IN *addr)
		: socket(sock), buffSize(MAX_BUFF_SIZE)
	{
		char tmp[33] = { 0 };
		ipAddress = (AF_INET, &(addr->sin_addr), tmp, sizeof(tmp));
		port = ntohs(addr->sin_port);
	}

	TCPConnection()
	{}

	TCPConnection(const TCPConnection& o)
	{}
};

class TCPConnector
{
public:

	TCPConnector()
	{}

	~TCPConnector()
	{}

	static TCPConnection* Connect(char *server, int port)
	{
		WSADATA wsaData;
		int ret = WSAStartup(MAKEWORD(2, 2), &wsaData);

		if (ret == NO_ERROR)
		{
			SOCKADDR_IN addr;
			int		newSocket;

			ZeroMemory(&addr, sizeof(addr));

			addr.sin_family = AF_INET;
			addr.sin_port = htons(port);
			InetPtonA(AF_INET, server, &(addr.sin_addr));
			newSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			ret = connect(newSocket, (SOCKADDR*)&addr, sizeof(addr));

			if (ret)
			{
				return NULL;
			}

			std::cout << "Connected" << std::endl;

			return new TCPConnection(newSocket, &addr);
		}
		return NULL;
	}
};

#endif