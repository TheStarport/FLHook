#pragma once
#include <FLHook.hpp>

#include "WinSock2.h"

class CSocket : public CCmds
{
  public:
	SOCKET s;
	BLOWFISH_CTX* bfc;
	bool bAuthed;
	bool bEventMode;
	bool bUnicode;
	bool bEncrypted;
	std::string sIP;
	ushort iPort;

	CSocket()
	{
		bAuthed = false;
		bEventMode = false;
		bUnicode = false;
	}
	void DoPrint(const std::wstring& wscText);
	std::wstring GetAdminName();
};
