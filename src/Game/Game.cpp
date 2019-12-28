#include "Game.h"
#include "Packet/Packet.h"
#include "../GameServer.h"
#include "Snapshot/Snapshot.h"
#include "Message/PlayerDisconnect.h"
#include "Message/PlayerChat.h"
#include "Message/RemoveEntity.h"
#include "Entity/Player.h"
#include "../Config.h"

#include <iostream>
#include <thread>
#include <stdint.h>



Game::Game(Socket* socket) : socket(socket) {
	for (int i = 255; i >= 0; --i) id_slots.push(static_cast<unsigned char>(i));
}

bool Game::connectClient(long long connection, InPacketInfo p_info) {
	if (connections_num >= config::MAX_CONNECTIONS) return false;
	std::cout << "Accepted connection " << static_cast<int>(connections_num) << "\n";

	Player client_entity = Player();
	unsigned char client_id = createEntity(std::make_shared<Player>(client_entity));

	connections[connection] = std::make_unique<Client>(
		this, client_id, p_info.sender_address, p_info.sender_port
	);

	connections_num++;

	return true;
}

unsigned char Game::createEntity(std::shared_ptr<Entity> entity) {
	unsigned char id = id_slots.top();
	id_slots.pop();

	snapshot_manager.master_snapshot.entities[id] = entity;

	return id;
}

void Game::removeEntity(unsigned char id) {
	RemoveEntity::Fields re_fields{ id };
	auto message = std::make_shared<RemoveEntity>(re_fields);

	for (auto& bc_client : connections) { // Broadcast the RemoveEntity message
		if (bc_client.second->id == id) continue;
		bc_client.second->reliable_queue.push(message);
	}

	// Remove the entity from the master snapshot
	auto player_state = snapshot_manager.master_snapshot.entities.find(id);
	if (player_state != snapshot_manager.master_snapshot.entities.end()) {
		snapshot_manager.master_snapshot.entities.erase(player_state->first);
	}

	// TODO:
	// We can't simply id_slots.push(id), we need to wait for the
	// reliable message to get acknowledged by all users.
}

void Game::receiveMessage(Client& client, InPacket& packet) {
	client.bump(); // Bump the client's last_received timer

	// Receive an unreliable command
	UnreliableCmd command = packet.read<UnreliableCmd>();
	switch (command) {
		case UnreliableCmd::Ping:
			break;
		case UnreliableCmd::PlayerData:
			snapshot_manager.updatePlayerState(client, packet); // Update master game state
			break;
		default:
			throw std::invalid_argument("Unknown unreliable command.");
	}

	client.ack(packet);
}

void Game::receiveReliableMessage(Client& client, InPacket& packet) {
	// Check if the received reliable message is a resend of previous
	bool unique_message = false;
	if (packet.reliable_switch == client.client_rel_switch) {
		unique_message = true;
		client.client_rel_switch = !client.client_rel_switch;
	}

	// Receive an unreliable command
	ReliableCmd command = packet.read<ReliableCmd>();
	switch (command) {
		case ReliableCmd::PlayerChat:
		{
			PlayerChat chat_message = PlayerChat(packet);
			if (unique_message) {
				chat_message.fields.entity_id = client.id;
				auto message = std::make_shared<PlayerChat>(chat_message);
				for (auto& i_client : connections) {
					i_client.second->reliable_queue.push(message);
				}
			}
			break;
		}
		default:
			throw std::invalid_argument("Unknown reliable command.");
	}

	receiveMessage(client, packet);
}

void Game::sendMessage(Client& client, OutPacket& packet) {
	packet.setHeaders(
		client.server_sequence, 
		client.sequences.empty ? (unsigned short)0 : client.sequences.last_sequence,
		client.sequences.ack_bitfield
	);

	client.server_sequence++; // Increase our sequence by one

	client.send(packet);
}

void Game::sendTickMessages() { // TODO: We should consider thread pools!!!!!
	std::vector<std::thread> threads;
	for (auto conn = connections.begin(); conn != connections.end(); ) { // Loop over clients
		if (conn->second->hasTimedOut()) {
			std::cout << "Connection " << static_cast<int>(conn->second->id) << " has timed out.\n";

			removeEntity(conn->second->id); // Remove the player entity
			conn = connections.erase(conn); // Delete the client instance
			connections_num--;
		} else {
			threads.emplace_back(
				std::thread(&Game::sendClientTick, this, std::ref(*conn->second.get()))
			);
			++conn;
		}
	}

	for (auto& th : threads) { // Wait for all tick threads to finish
		th.join();
	}
}

void Game::sendClientTick(Client& client) {
	bool send_reliable = client.shouldSendReliable();

	OutPacket tick_packet = OutPacket( // We might not need a packet type at all in the future
		send_reliable ? PacketType::Reliable : PacketType::Unreliable, 
		client.buffer,
		send_reliable ? client.server_rel_switch : false
	);

	// Write optional reliable message
	if (send_reliable) client.appendReliable(tick_packet);

	// Write snapshot message
	tick_packet.write(UnreliableCmd::Snapshot);
	snapshot_manager.writeSnapshot(client, tick_packet);

	sendMessage(client, tick_packet); // Send the packet
}
