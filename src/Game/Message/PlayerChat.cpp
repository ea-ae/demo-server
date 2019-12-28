#include "PlayerChat.h"
#include "../Game.h"


PlayerChat::PlayerChat(InPacket& packet) {
	read(packet);
};

PlayerChat::PlayerChat(Fields& data) : fields(data) {}

void PlayerChat::read(InPacket& packet) {
    fields.message_length = packet.read<unsigned char>();
    fields.chat_message = packet.read_string(fields.message_length);
}

void PlayerChat::serialize(OutPacket& packet) {
	packet.write(static_cast<unsigned char>(ReliableCmd::PlayerChat));
	packet.write(fields.entity_id);
    packet.write(fields.message_length);
    packet.write(fields.chat_message);
}

void PlayerChat::on_ack(Client&) {}

void PlayerChat::on_fail(Client&) {}
