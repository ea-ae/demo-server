#include "InPacket.h"

#include <iostream>
#include <stdint.h>
//#include <bitset>


InPacket::InPacket(unsigned char buffer_in[], unsigned short import_size) {
	buffer = buffer_in;

	if (import_size > 0) {
		build(import_size);
	}
};

template<> UnreliableCmd InPacket::read<UnreliableCmd>() {
	increase_buffer_index(1);
	return static_cast<UnreliableCmd>(buffer[buffer_index - 1]);
};

template<> ReliableCmd InPacket::read<ReliableCmd>() {
	increase_buffer_index(1);
	return static_cast<ReliableCmd>(buffer[buffer_index - 1]);
};

template<> ControlCmd InPacket::read<ControlCmd>() {
	increase_buffer_index(1);
	return static_cast<ControlCmd>(buffer[buffer_index - 1]);
};

template<> unsigned char InPacket::read<unsigned char>() {
	increase_buffer_index(1);
	return buffer[buffer_index - 1];
};

template<> unsigned short InPacket::read<unsigned short>() {
	increase_buffer_index(2);
	return (static_cast<unsigned short>(buffer[buffer_index - 2]) << 8) | buffer[buffer_index - 1];
};

template<> uint32_t InPacket::read<uint32_t>() {
	increase_buffer_index(4);
	return (uint32_t)buffer[buffer_index - 4] << 24 |
		(uint32_t)buffer[buffer_index - 3] << 16 |
		(uint32_t)buffer[buffer_index - 2] << 8 |
		(uint32_t)buffer[buffer_index - 1];
};

template<> int32_t InPacket::read<int32_t>() {
	return (int32_t)read<uint32_t>();
};

std::string InPacket::read_string(unsigned short size, bool encode) {
	if (encode) throw std::exception("String encoding not implemented!");

	increase_buffer_index(size);
	return std::string(&buffer[buffer_index - size], &buffer[buffer_index]);
}

void InPacket::build(unsigned short buffer_size) {
	packet_length = buffer_size;

	BasePacketHeader header;
	header.raw = read<unsigned char>();
	packet_type = header.fields.packet_type;
	reliable_switch = header.fields.reliable_switch;

	unsigned short packet_length_header = read<unsigned short>(); // todo: get rid of this header

	if (packet_length != packet_length_header) {
		throw std::exception("Packet length not equal to import size.");
	}

	if (packet_type != PacketType::Control) {
		packet_sequence = read<unsigned short>();
		packet_ack = read<unsigned short>();
		ack_bitfield = read<uint32_t>();
	}

	/*std::cout << "PACKET RECEIVED\n";
	for (int i = 0; i < buffer_size; i++) {
		std::cout << std::bitset<8>(buffer[i]).to_string() << " ";
	}
	std::cout << "\n";*/
}

void InPacket::increase_buffer_index(unsigned short amount) {
	buffer_index += amount;
	if (buffer_index > packet_length) {
		throw std::out_of_range("Buffer index exceeds packet size.");
	}
}
