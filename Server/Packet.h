#ifndef _PACKET_H_
#define _PACKET_H_

#include <string>
#include <vector>

#define MTU 1024 //maximum transmission unit for this protocol

typedef unsigned char byte;

static const byte SYNC_BYTE = 0x99;
static const size_t _SIZEOFINT = sizeof(int);
static const size_t _SIZEOFSHORT = sizeof(short);
static const size_t HEADER_SIZE = 1 + (_SIZEOFSHORT * 2) +_SIZEOFINT;

enum Opcode
{
	//define packet types here
	NOP = 0xfe,
	CONNECT = 0x0,
	DISCONNECT = 0x1,
};

#pragma pack(1)
struct _header
{
	byte sync = SYNC_BYTE; //byte to indicate start of packet
	uint16_t opcode = NOP; //packet identifier
	uint16_t size = HEADER_SIZE; //size of total packet (including header)
	uint32_t checksum = HEADER_SIZE ^ 42; //TODO: actual checksum algorithm
};

struct _packet
{
	_header header;
	std::vector<byte> data; //variable length payload
};

/*
**	Functions used to push values into a packet's data.
**	The size of a packet and its checksum 
**	must be recalculated after modifying the data.
*/
void writeShort(_packet *packet, short val);
void writeInt32(_packet *packet, int val);
void writeString(_packet *packet, const char* val);
void writeWString(_packet *packet, const wchar_t* val);

_packet readPacket(const byte *buffer); //get packet from byte array
void writePacket(byte *buffer, size_t buffSize, _packet *packet); //copy packet to buffer

#endif