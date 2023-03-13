#include <FLHook.hpp>

// Common packets are being sent from server to client and from client to server.
FlPacket* FlPacket::Create(uint size, CommonPacket kind)
{
	auto* packet = static_cast<FlPacket*>(malloc(size + 6));
	packet->size = size + 2;
	packet->type = 1;
	packet->kind = kind;

	return packet;
}

// Server packets are being sent only from server to client.
FlPacket* FlPacket::Create(uint size, ServerPacket kind)
{
	auto* packet = static_cast<FlPacket*>(malloc(size + 6));
	packet->size = size + 2;
	packet->type = 2;
	packet->kind = kind;

	return packet;
}

// Client packets are being sent only from client to server. Can't imagine why you ever need to create such a packet at side of server.
FlPacket* FlPacket::Create(uint size, ClientPacket kind)
{
	auto* packet = static_cast<FlPacket*>(malloc(size + 6));
	packet->size = size + 2;
	packet->type = 3;
	packet->kind = kind;

	return packet;
}

// Returns true if sent succesfully, false if not. Frees memory allocated for packet.
bool FlPacket::SendTo(ClientId client)
{
	CDPClientProxy* cdpClient = clientProxyArray[client - 1];

	// We don't include first 4 bytes directly in packet, it is info about size. Type and kind are already included.
	const bool result = cdpClient->Send((byte*)this + 4, size);

	// No mistakes, free allocated memory.
	free(this);

	return result;
}
