#include "Server.h"

int main()
{
	Server server(SERVER_PORT);
	server.run();
    return 0;
}

