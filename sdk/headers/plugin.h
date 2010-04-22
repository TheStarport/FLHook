#ifndef __PLUGIN_H__
#define __PLUGIN_H__ 1

enum PLUGIN_RETURNCODE
{
	DEFAULT_RETURNCODE = 0,
	SKIPPLUGINS = 1,
	SKIPPLUGINS_NOFUNCTIONCALL = 2,
	NOFUNCTIONCALL = 3,
};

enum PLUGIN_MESSAGE
{
	DEFAULT_MESSAGE = 0,
	CONDATA_EXCEPTION = 10,
	CONDATA_DATA = 11,
	TEMPBAN_BAN = 20,
	ANTICHEAT_TELEPORT = 30,
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
// message structs

struct ANTICHEAT_TELEPORT_STRUCT // in
{
	uint iClientID; 
	Vector vNewPos; 
	Matrix mNewOrient; 
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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
__declspec(dllimport) void Plugin_Communication(PLUGIN_MESSAGE msgtype, void* msg);
extern __declspec(dllimport) bool g_bPlugin_nofunctioncall;

#endif