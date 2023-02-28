#pragma once

#include <string>

using uint = unsigned int;
using uchar = unsigned char;
using ushort = unsigned short;
using ulong = unsigned long;
using int64 = long long;
using uint64 = unsigned long long;
using mstime = uint64;

// Common types that can be used to explain what is being used

using ClientId = const uint;
using SystemId = const uint;
using ShipId = const uint;
using EquipId = const uint;
using BaseId = const uint;
using RepId = const ushort;
using GoodId = const uint;
using ArchId = const uint;

using st6_malloc_t = void* (*)(size_t);
using st6_free_t = void (*)(void*);

using _RCSendChatMsg = void(__stdcall*)(uint iId, uint iTo, uint iSize, void* pRDL);
using _CRCAntiCheat = void(__stdcall*)();
using _GetFLName = int(__cdecl*)(char* szBuf, const wchar_t* wszStr);
using _GetShipInspect = bool(__cdecl*)(uint& ship, class IObjInspectImpl*& inspect, uint& iDunno);

using BLOWFISH_CTX = struct
{
	unsigned long P[16 + 2];
	unsigned long S[4][256];
};

using BYTE = unsigned char;

using UserCmdProc = void(*)(ClientId& client);
using UserCmdProcWithParam = void(*)(ClientId& client, const std::wstring& param);
