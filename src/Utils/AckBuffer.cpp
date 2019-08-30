#include "AckBuffer.h"

#include <iostream>
#include <bitset>


AckBuffer::AckBuffer() {}

void AckBuffer::put(unsigned short sequence) {
	int difference;
	bool overflow;

	if (empty) {
		last_sequence = sequence;
		empty = false;
		return;
	}

	// Check for overflow
	if (sequence - last_sequence <= 32768) {
		overflow = false;
	} else if (last_sequence - sequence > 32768) {
		overflow = true;
	}

	// Check if new sequence is larger than the previous & get difference
	if ((sequence > last_sequence) && !overflow) {
		difference = sequence - last_sequence;
	} else if ((sequence < last_sequence) && overflow) {
		difference = sequence - last_sequence + 65536; // Overflow
	} else {
		// Sequence is smaller, insert it into bitfield
		difference = overflow ? abs(sequence - last_sequence - 65536) : last_sequence - sequence;

		if (difference > 0 && difference <= 32) {
			ack_bitfield = ack_bitfield | (uint32_t)pow(2, difference - 1);
		}

		return;
	}

	ack_bitfield = ack_bitfield << difference;
	ack_bitfield = ack_bitfield + (uint32_t)pow(2, difference - 1);

	// std::cout << "seq " << sequence << " last_seq " << last_sequence << " diff " << difference << " <> " << std::bitset<32>(ack_bitfield).to_string() << "\n\n";

	last_sequence = sequence;
}

void AckBuffer::reset() {
	empty = true;
	ack_bitfield = 0;
}
