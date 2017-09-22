#include "Server.h"

static int id = 1;
static std::basic_ofstream<wchar_t> logger;
static std::mutex gMutex;

//THREAD ROUTINES
void cli_routine(LPVOID param)
{
	Server *server = ((Server*)param);
	std::string input;
	std::string word;
	std::vector<std::string> params;

	while (true)
	{
		std::cout << "> ";
		std::getline(std::cin, input);
		std::stringstream ss(input);

		while (ss >> word)
		{
			params.push_back(word);
		}

		if (params.size() > 0 && params.size() < 4)
			server->executeCommand(params);
		params.clear();
	}
}

/*
//thread to accept incoming connections
void accept_routine(LPVOID param)
{
	Server *server = (Server*)param;
	int socket = server->getSocket();
	fd_set set;

	while (server->isRunning())
	{
		FD_ZERO(&set);
		FD_SET(socket, &set);

		int	ret = select(FD_SETSIZE, &set, NULL, NULL, NULL);

		if (FD_ISSET(socket, &set))
		{
			sockaddr_in cAddr;
			int addlen = sizeof(cAddr);
			int newSocket = accept(socket, (sockaddr*)&cAddr, &addlen);
			Client client(id++, newSocket, cAddr);
			server->addClient(client);
		}
	}
}
*/

size_t handlePackets(Client *c)
{
	if (c->incomingPackets.size() == 0)
		return 0;

	auto pPackets = &c->incomingPackets;
	size_t count = 0;

	for (auto &&p : c->incomingPackets)
	{
		++count;
		std::cout << "received packet 0x" << std::hex << p.header.opcode << " from client " << c->id << std::endl;
	}

	c->incomingPackets.clear();

	return count;
}

bool parsePacket(Client *c, byte *buffer)
{
	_header *pHeader = (_header*)(buffer);

	//check for packet integrity (WIP)
	if (pHeader->checksum != ((pHeader->size + pHeader->opcode) ^ 42))
		return false;

	_packet p = readPacket(buffer);
	c->enqueuePacket(p);

	return true;
}

//read tcp stream for packets
size_t readNextPackets(Client *c, byte *buffer, size_t size)
{
	size_t total = size;

	while (size > 0)
	{
		size_t i = 0;
		//loop through buffer until sync byte is found indicating beginning of packet
		while (i < size)
		{
			if ((byte)buffer[i] == SYNC_BYTE) //sync byte
			{
				if (i > 0)
					memmove(buffer, buffer + i, size - i);
				size -= i;
				break;
			}
			++i;
		}

		//no sync byte was found reset buffer
		if (i == size)
			return 0;

		_header *pHeader = (_header*)(buffer);
		size_t packetSize = pHeader->size;
		uint16_t opcode = pHeader->opcode;

		if (packetSize <= size)
		{
			parsePacket(c, buffer);
			//remove packet that was read from buffer
			memmove(buffer, buffer + pHeader->size, size - packetSize);
			size -= packetSize;
		}
		else
		{
			//read more from packet if it's smaller than buffer size else discard it
			return ((size < total) ? size : 0);
		}
	}
	//finished reading packets, reset buffer
	return 0;
}

void client_routine(LPARAM param)
{
	Client c = *((Client*)param);
	byte rcvBuffer[MAX_RECV_BUF] = { 0 };

	std::cout << c.getIp() << ":" << c.addr.sin_port << " connected" << std::endl;

	int socket = c.socket;
	size_t total = 0;
	int count = 0;

	while (c.connected)
	{
		int bytesReceived =
			recv(socket, (char*)(rcvBuffer + total), sizeof(rcvBuffer) - total, 0);
		if (bytesReceived > 0)
		{
			total += bytesReceived;
			total = readNextPackets(&c, rcvBuffer, total);
			handlePackets(&c);
		}
		else
			c.connected = false;
	}

	std::cout
		<< c.getIp() << ":" << c.addr.sin_port << " disconnected"
		<< std::endl;
	ExitThread(1);
}

Server::Server(short port)
{
	if (!init(port))
	{
		closesocket(socket);
		WSACleanup();
		exit(1);
	}
}

Server::~Server()
{
	CloseHandle(thCliRoutine);
	closesocket(socket);
	WSACleanup();
}

void Server::addClient(const Client client)
{
	for (auto &c : clients)
	{
		if (c.id == client.id) return;
	}

	clients.push_back(client);

	//create thread to handle client connection
	HANDLE tHandle = CreateThread(
		NULL, 0, (LPTHREAD_START_ROUTINE)&client_routine,
		(void*)&clients.back(), 0, NULL);

	if (tHandle == NULL)
	{
		std::cerr << "error creating client thread" << std::endl;
		clients.pop_back();
	}
}

void Server::removeClient(int id)
{
	std::lock_guard<std::mutex> mutex(gMutex);
	clients.erase(std::remove_if(clients.begin(), clients.end(), [&](const Client& c)
	{
		return c.id == id;
	}), clients.end());
}

bool Server::init(short port)
{
	WSAData data;

	int ret = WSAStartup(MAKEWORD(2, 2), &data);
	if (ret)
	{
		std::cerr << "WSASTARTUP FAILED : " << WSAGetLastError() << std::endl;
		return false;
	}

	this->socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (this->socket < 0)
	{
		std::cerr << std::strerror(errno) << std::endl;
		return false;
	}
	/*
	char optVal = 1;
	if (setsockopt(this->socket, SOL_SOCKET, SO_REUSEADDR, &optVal, sizeof(int)) != 0)
	{
		std::cout << strerror(errno) << std::endl;
		return false;
	}
	*/
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	ret = bind(this->socket, (struct sockaddr*)&addr, sizeof(addr));
	if (ret < 0)
	{
		std::cerr << std::strerror(errno) << std::endl;
		return false;
	}

	ret = listen(this->socket, SOMAXCONN);
	if (ret < 0)
	{
		std::cerr << std::strerror(errno) << std::endl;
		return false;
	}

	thCliRoutine = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&cli_routine, this, 0, NULL);
	if (thCliRoutine == NULL)
	{
		std::cerr << "Error creating CLI routine" << std::endl;
		return false;
	}

	return true;
}

void Server::run()
{
	sockaddr_in cAddr;
	int addlen = sizeof(cAddr);
	this->running = true;

	//listen for new connections
	while (this->running)
	{
		sockaddr_in cAddr;
		int addlen = sizeof(cAddr);
		int newSocket = accept(socket, (sockaddr*)&cAddr, &addlen);
		addClient(Client(id++, newSocket, cAddr));
	}
}

bool Server::stop()
{
	return false;
}

bool Server::executeCommand(std::vector<std::string>& args)
{
	if (args.empty())
		return true;

	if (!args[0].compare("/help") ||
		!args[0].compare("?"))
	{
		std::cout << "\nList of available commands:\n\n";
		std::cout << "/all\t\tLists all connected clients\n";
		std::cout << "/kick {id}\tCloses connection to client\n";
		std::cout << "/ban {id}\tAdds specified client to blacklist\n";
		std::cout << "/whois {id}\tShows additional informations about a client\n";
		std::cout << "/msg {id}\tSends a message to client\n";
		std::cout << std::endl;

		return true;
	}
	
	if (!args[0].compare("/all"))
	{
		std::cout << "\nClients connected:\t" << clients.size() << "\n\n";

		for (auto &&c : clients)
		{
			std::cout << c << "\n";
		}

		std::cout << std::endl;

		return true;
	}

	std::cout << "Unknown command " << args[0] << std::endl;
		 
	return false;
}
