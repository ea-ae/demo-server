#pragma once

#include "Entity.h"
#include "../Packet/InPacket.h"
#include "../Packet/OutPacket.h"

#include <memory>


class Player : public Entity {
public:
	struct State {
		int32_t pos_x = 0;
		int32_t pos_y = 0;
		uint8_t score = 0;
	};

	enum class Fields : unsigned char {
		PosX,
		PosY,
		Score,
		End
	};

	union ModFields {
		struct {
			bool pos_x : 1;
			bool pos_y : 1;
			bool score : 1;
			bool empty : 5;
		} fields;
		unsigned char raw;
	};

	State entity_state;
private:
	static const State dummy_state;
public:
	Player();
	Player(State& state);
	void read(InPacket& packet) override;
	void serialize(OutPacket& packet) override;
	void serialize(OutPacket& packet, Entity& last_entity) override;
	void serialize(OutPacket& packet, const State& last_state);
};
