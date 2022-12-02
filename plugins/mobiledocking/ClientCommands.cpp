#include "Main.h"

/// Send a command to the client at destination Id 0x9999
void SendCommand(ClientId client, const std::wstring& message)
{
	FMsg(client, L"<TEXT>" + XMLText(message) + L"</TEXT>");
}

void SendSetBaseEntryText(UINT client, const std::wstring& message)
{
	SendCommand(client, std::wstring(L" SetBaseEntryText ") + message);
}

void SendSetSystemEntryText(UINT client, const std::wstring& message)
{
	SendCommand(client, std::wstring(L" SetSystemEntryText ") + message);
}

void SendSetBaseInfoText(UINT client, const std::wstring& message)
{
	SendCommand(client, std::wstring(L" SetBaseInfoText ") + message);
}

void SendSetBaseInfoText2(UINT client, const std::wstring& message)
{
	SendCommand(client, std::wstring(L" SetBaseInfoText2 ") + message);
}

void SendSetOverrideSystem(UINT client, UINT iSystemId, UINT iVirtualBaseId)
{
	wchar_t buf[200];
	_snwprintf_s(buf, sizeof(buf), L" SetOverrideSystem %u %u", iSystemId, iVirtualBaseId);
	SendCommand(client, buf);
}

void SendResetMarketOverride(UINT client)
{
	SendCommand(client, L" ResetMarketOverride");
	SendCommand(client, L" SetMarketOverride 0 0 0 0");
}
