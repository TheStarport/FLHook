#pragma once

struct DLL CoreGlobals : Singleton<CoreGlobals>
{
	uint damageToClientId {};
	uint damageToSpaceId {};

	bool messagePrivate {};
	bool messageSystem {};
	bool messageUniverse {};

	std::wstring accPath;

	uint serverLoadInMs {};
	uint playerCount {};
	bool disableNpcs {};

	std::list<BaseInfo> allBases;

	bool flhookReady {};
};
