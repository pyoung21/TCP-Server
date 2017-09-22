#include "TCPConnection.h"
#include "Packet.h"

#define THROTTLING_DELAY 1000 //delay in ms to send data
#define SERVER_PORT 5000

int main()
{
	_packet a, b, c;

	a.header.opcode = 0x45;
	a.header.checksum = (0x45 + 9) ^ 42;
	b.header.opcode = 0x88;
	b.header.checksum = (0x88 + 9) ^ 42;
	c.header.opcode = 0x22;
	c.header.checksum = (0x22 + 9) ^ 42;

	auto *cnx = TCPConnector::Connect("127.0.0.1", SERVER_PORT); //local server

	byte buffer[512] = { 0 };

	writePacket(buffer, sizeof(buffer), &a);
	writePacket(buffer + 50, sizeof(buffer), &b);
	writePacket(buffer + 100, sizeof(buffer), &b);
	writePacket(buffer + 150, sizeof(buffer), &a);

	while (true)
	{
		*cnx << a << b << c << b << a << c << a << a;
		cnx->sendBytes((const char*)buffer, sizeof(buffer));
		Sleep(THROTTLING_DELAY);
	}

	return 0;
}
