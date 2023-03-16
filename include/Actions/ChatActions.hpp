#pragma once

class DLL ChatActions
{
	static void SendFormatted(ClientId client, char* Buf, uint size);
	static void SendGroup(uint fromClientId, const std::wstring& text);
	static void SendLocalSystem(uint fromClientId, const std::wstring& text);
	static void SendSystemChat(uint fromClientId, const std::wstring& text);
};