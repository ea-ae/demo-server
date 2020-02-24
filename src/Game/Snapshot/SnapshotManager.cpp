#include "SnapshotManager.h"
#include "../Entity/Player.h"
#include "../../Config.h"

#include <iostream>
#include <unordered_map>
#include <string>


SnapshotManager::SnapshotManager() {}

bool SnapshotManager::writeSnapshot(Client& client, OutPacket& packet) {
	if (config::DEBUG) std::cout << "[Snapshot Info]\n";
	if (config::DEBUG) std::cout << "[SID]\t" << static_cast<int>(client.server_sequence) << "\n";

	// Create a new delta-compressed snapshot
	std::shared_ptr<Snapshot> new_snapshot = std::make_shared<Snapshot>(client.server_sequence);
	//packet.packet_sequence = client.server_sequence; // VERY TEMP!!!!!!!!!!!!

	// Get the latest snapshot acked by the client
	std::shared_ptr<Snapshot> last_snapshot = client.snapshots.get(client.last_snapshot);

	// Write modified entity data
	if (!writeDelta(packet, last_snapshot == nullptr ? nullptr : last_snapshot.get(), client.id)) {
		return false; // No entities changed since last snapshot, don't save it
	}

	// Copy master entity states into our new snapshot
	new_snapshot->entities = master_snapshot.entities;

	// Add the new snapshot to the client's SnapshotBuffer
	client.snapshots.add(new_snapshot);

	return true;
}

// pass shared_ptr's instead of raw pointers? gotta figure out why we did this in the first place
bool SnapshotManager::writeDelta(OutPacket& packet, Snapshot* last_snapshot, unsigned char entity_id) {
	bool empty_snapshot = true;

	// Iterate over all entities in the master snapshot

	for (auto entity = master_snapshot.entities.begin(); entity != master_snapshot.entities.end(); ++entity) {
		if (entity->first == entity_id) {
			continue; // Don't send the client information about itself
		}

		unsigned short pre_entity_index = packet.getBufferIndex();
		packet.write(entity->first); // Write the entity ID
		unsigned short entity_index = packet.getBufferIndex(); // in case we add int16 entity id's

		// Write the entity's delta fields
		if (last_snapshot == nullptr) { // Last snapshot doesn't exist anymore (too old), send all of the fields
			entity->second->serialize(packet); // If last_entity is nullptr, it's guaranteed to be sent again
		} else {
			// Find the given entity in last_snapshot
			auto last_entity_it = last_snapshot->entities.find(entity->first);
			if (last_entity_it != last_snapshot->entities.end()) {
				entity->second->serialize(packet, *last_entity_it->second);
			} else { // Entity not found
				entity->second->serialize(packet);
			}
		}

		if (entity_index == packet.getBufferIndex()) { // Don't write IDs of unchanged entities
			packet.setBufferIndex(pre_entity_index);
		} else {
			empty_snapshot = false;
		}

		if (config::DEBUG) std::cout << "[EID]\t" << static_cast<int>(entity->first) << "\n";
	}

	return !empty_snapshot;
}
