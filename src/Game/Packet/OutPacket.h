#pragma once

#include "Packet.h"

#include <stdint.h>


class OutPacket : public Packet {
public:
	OutPacket(PacketType type, unsigned char buffer_in[]);

	void setHeaders(unsigned short sequence, unsigned short ack, uint32_t ack_bitfield);
	void setPacketLength();

	void write(ControlCmd value);
	void write(UnreliableCmd value);
	void write(unsigned char value);
	void write(unsigned short value);
	void write(uint32_t value);
};