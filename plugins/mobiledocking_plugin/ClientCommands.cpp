#include "main.h"

/// Send a command to the client at destination ID 0x9999
void SendCommand(uint client, const std::wstring& message)
{
	HkFMsg(client, L"<TEXT>" + XMLText(message) + L"</TEXT>");
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

void SendSetOverrideSystem(UINT client, UINT iSystemID, UINT iVirtualBaseID)
{
	wchar_t buf[200];
	_snwprintf_s(buf, sizeof(buf), L" SetOverrideSystem %u %u", iSystemID, iVirtualBaseID);
	SendCommand(client, buf);
}

void SendResetMarketOverride(UINT client)
{
	SendCommand(client, L" ResetMarketOverride");
	SendCommand(client, L" SetMarketOverride 0 0 0 0");
}
