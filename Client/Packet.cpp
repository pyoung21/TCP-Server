#include "Packet.h"

void writeShort(_packet *packet, short val)
{
	packet->data.resize(packet->data.size() + _SIZEOFSHORT);
	memcpy(&packet->data[packet->data.size() - _SIZEOFSHORT], &val, _SIZEOFSHORT);
	packet->header.size += _SIZEOFSHORT;
	packet->header.checksum = (packet->header.size + packet->header.opcode) ^ 42;
}

void writeInt32(_packet *packet, int val)
{
	packet->data.resize(packet->data.size() + _SIZEOFINT);
	memcpy(&packet->data[packet->data.size() - _SIZEOFINT], &val, _SIZEOFINT);
	packet->header.size += _SIZEOFINT;
	packet->header.checksum = (packet->header.size + packet->header.opcode) ^ 42;
}

void writeString(_packet *packet, const char* val)
{
	std::string s(val);
	packet->data.resize(packet->data.size() + s.size() + 1);
	memcpy(&packet->data[packet->data.size() - s.size() - 1], s.c_str(), s.size());
	packet->header.size += s.size() + 1;
	packet->header.checksum = (packet->header.size + packet->header.opcode) ^ 42;
}

void writeWString(_packet *packet, const wchar_t* val)
{
	std::wstring s(val);
	packet->data.resize(packet->data.size() + s.size() * sizeof(wchar_t) + 1);
	memcpy(
		&packet->data[packet->data.size() - s.size() * sizeof(wchar_t) - 1],
		s.c_str(), s.size() * sizeof(wchar_t));
	packet->header.size += s.size() * sizeof(wchar_t) + 1;
	packet->header.checksum = (packet->header.size + packet->header.opcode) ^ 42;
}

_packet readPacket(const byte *buffer)
{
	_packet packet;
	_header *pHeader = (_header*)(buffer);
	size_t dataSize = pHeader->size - HEADER_SIZE;

	packet.header = *(_header*)(buffer);
	packet.data.resize(dataSize, 0);
	memcpy(packet.data.data(), buffer + HEADER_SIZE, dataSize);

	return packet;
}

void writePacket(byte *buffer, size_t buffSize, _packet *packet)
{
	if (packet->header.size > buffSize)
		return;
	else
	{
		memcpy(&buffer[0], &packet->header, HEADER_SIZE);
		memcpy(&buffer[HEADER_SIZE], packet->data.data(), packet->data.size());
	}
}