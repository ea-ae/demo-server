#include "Game.h"
#include "Game/Packet/Packet.h"
#include "GameServer.h"
#include "Game/Snapshot/Snapshot.h"

#include <iostream>
#include <stdint.h>


/*Game::Game(GameServer* gameServer) :
	server(gameServer) {}*/

Game::Game(Socket* socket) : socket(socket) {}

Client* Game::connRequest(unsigned long ip, unsigned short port) {
	if (connections_num < MAX_CONNECTIONS) {
		// Once we implement disconnection, the ID system will be improved
		
		Client* client = new Client(this, connections_num, ip, port);

		std::cout << "Client id connRequest: " << static_cast<uint16_t>(client->id) << "\n";

		//clients[connections] = client;
		clients.emplace_back(client);

		connections_num++;

		return clients.back();
	} else {
		throw std::exception("Connection limit reached.");
	}
}

void Game::receiveCommand(Client& client, InPacket& packet) {
	std::cout << "Sequence " << packet.packet_sequence << 
	" Ack " << packet.packet_ack << " AckBitfield " << packet.ack_bitfield << "\n";

	std::cout << "Client id recvCommand: " << static_cast<uint16_t>(client.id) << "\n";

	// Receive an unreliable command
	UnreliableCmd command = packet.read<UnreliableCmd>();

	switch (command) {
		case UnreliableCmd::PlayerData: // rename to PlayerData or something like that
			// Update master game state
			snapshot_manager.updatePlayerState(packet, client);
			break;
		default:
			throw std::invalid_argument("Unknown command.");
	}

	client.sequences.put(packet.packet_sequence); // Update ack
}

void Game::sendCommand(Client& client, OutPacket& packet) {
	// Set the headers
	packet.setHeaders(
		client.server_sequence, 
		client.sequences.empty ? (unsigned short)0 : client.sequences.last_sequence,
		client.sequences.ack_bitfield
	);

	// Increase our sequence by one
	client.server_sequence++;

	//server->send(packet, client.ip, client.port);
	client.send(packet);
}

void Game::sendSnapshots() {
	// Iterate over all clients
	for (unsigned char i = 0; i < connections_num; ++i) {
		OutPacket ss_packet = OutPacket(PacketType::Unreliable, buffer);
		ss_packet.write(UnreliableCmd::Snapshot);

		snapshot_manager.writeSnapshot(ss_packet, *clients[i]); // Write snapshot data

		sendCommand(*clients[i], ss_packet); // Send the packet
	}
}
