#ifndef __PLUGIN_H__
#define __PLUGIN_H__ 1

enum PLUGIN_RETURNCODE
{
	DEFAULT_RETURNCODE = 0,
	SKIPPLUGINS = 1,
	SKIPPLUGINS_NOFUNCTIONCALL = 2,
	NOFUNCTIONCALL = 3,
};

struct PLUGIN_INFO
{
	string sName;
	string sShortName;
	bool bMayPause;
	bool bMayUnload;
	std::map<string, int> mapHooks;
};

enum PLUGIN_MESSAGE
{
	DEFAULT_MESSAGE = 0,
	CONDATA_EXCEPTION = 10,
	CONDATA_DATA = 11,
	TEMPBAN_BAN = 20,
	ANTICHEAT_TELEPORT = 30,
	ANTICHEAT_CHEATER = 31,
	DSACE_CHANGE_INFOCARD = 40,
	DSACE_SPEED_EXCEPTION = 41
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// message structs

struct ANTICHEAT_TELEPORT_STRUCT // in
{
	uint iClientID; 
	Vector vNewPos; 
	Matrix mNewOrient; 
};

enum ANTICHEAT_CHEAT_CODE
{
	AC_CODE_POWER,
	AC_CODE_TIMER,
	AC_CODE_SPEED,
	AC_CODE_MINING,
};

struct ANTICHEAT_CHEATER_STRUCT
{
	uint iClientID; 
	wstring wscCharname;
	ANTICHEAT_CHEAT_CODE CheatCode;
	wstring wscLog;
	bool bKilled;
};

struct CONDATA_DATA_STRUCT
{
	uint		iClientID; // in
	uint		iAveragePing; // out
	uint		iAverageLoss; // out
	uint		iPingFluctuation; // out
	uint		iLags; // out
};

struct	CONDATA_EXCEPTION_STRUCT // in
{
	uint iClientID;
	bool bException;
    string sReason;
};

struct	TEMPBAN_BAN_STRUCT // in
{
	uint iClientID;
    uint iDuration;
};

struct DSACE_CHANGE_INFOCARD_STRUCT
{
	uint iClientID;
	uint ids;
	wstring text;
};

struct DSACE_SPEED_EXCEPTION_STRUCT
{
	uint iClientID;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
__declspec(dllimport) void Plugin_Communication(PLUGIN_MESSAGE msgtype, void* msg);
extern __declspec(dllimport) bool g_bPlugin_nofunctioncall;

#endif