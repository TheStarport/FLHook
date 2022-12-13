#include <FLHook.hpp>

// Common packets are being sent from server to client and from client to server.
FLPACKET* FLPACKET::Create(uint size, FLPACKET::COMMON kind)
{
	auto* packet = static_cast<FLPACKET*>(malloc(size + 6));
	packet->Size = size + 2;
	packet->type = 1;
	packet->kind = kind;

	return packet;
}

// Server packets are being sent only from server to client.
FLPACKET* FLPACKET::Create(uint size, FLPACKET::SERVER kind)
{
	auto* packet = static_cast<FLPACKET*>(malloc(size + 6));
	packet->Size = size + 2;
	packet->type = 2;
	packet->kind = kind;

	return packet;
}

// Client packets are being sent only from client to server. Can't imagine why you ever need to create such a packet at side of server.
FLPACKET* FLPACKET::Create(uint size, FLPACKET::CLIENT kind)
{
	auto* packet = static_cast<FLPACKET*>(malloc(size + 6));
	packet->Size = size + 2;
	packet->type = 3;
	packet->kind = kind;

	return packet;
}

// Returns true if sent succesfully, false if not. Frees memory allocated for packet.
bool FLPACKET::SendTo(ClientId client)
{
	CDPClientProxy* cdpClient = g_cClientProxyArray[client - 1];

	// We don't include first 4 bytes directly in packet, it is info about size. Type and kind are already included.
	bool result = cdpClient->Send((byte*)this + 4, Size);

	// No mistakes, free allocated memory.
	free(this);

	return result;
}