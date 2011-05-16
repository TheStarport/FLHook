//////////////////////////////////////////////////////////////////////
//	Project FLCoreSDK v1.1, modified for use in FLHook Plugin version
//--------------------------
//
//	File:			FLCoreRemoteClient.h
//	Module:			FLCoreRemoteClient.lib
//	Description:	Interface to RemoteClient.dll
//
//	Web: www.skif.be/flcoresdk.php
//  
//
//////////////////////////////////////////////////////////////////////
#ifndef _FLCOREREMOTECLIENT_H_
#define _FLCOREREMOTECLIENT_H_

#include "FLCoreDefs.h"
#include <vector>

#pragma comment( lib, "FLCoreRemoteClient.lib" )

class IMPORT IClient
{
public:
	 IClient(class IClient const &);
	 IClient(void);
	 class IClient & operator=(class IClient const &);

public:
	unsigned char data[OBJECT_DATA_SIZE];
};


class CDPServer;

///////////////////////////////
///// FL PACKET STRUCTS ///////
///////////////////////////////

struct FLPACKET_UNKNOWN
{
	uint iDunno[20];
};

struct FLPACKET_LAUNCH
{
	uint iShip;
	uint iBase;
	uint iState;
	float fRotate[4];
	float fPos[3];
};

struct FLPACKET_BURNFUSE
{
	uint iShipAttacker;
	uint iShip;
	uint iFuseID;
	uint iDunno1;
	uint iDunno2;
	float fDunno;
	bool bActive;
};

struct FLPACKET_DESTROYOBJECT
{
	uint iSpaceID;
	uint iDestroyType;
};

struct FLPACKET_CREATESHIP
{
	char* pAddress; // ??
	uint iSpaceID;
	uint iShipArch;
	uint iDunno[2];
	float fDunno[14];
	float fPosX;
	float fPosY;
	float fPosZ;
	float fDunno2[13];
};

struct FLPACKET_LAND
{
	uint iShip;
	uint iLandSpaceID;
	uint iTargetBase;
};

struct HkIClientImpl
{
public:
	void* pDunno[2];
	CDPServer* cdpserver;
	virtual bool Send_FLPACKET_COMMON_FIREWEAPON(uint iDunno, FLPACKET_UNKNOWN& pDunno);
	virtual bool Send_FLPACKET_COMMON_ACTIVATEEQUIP(uint iClientID, struct XActivateEquip& aq);
	virtual bool Send_FLPACKET_COMMON_ACTIVATECRUISE(uint iClientID, XActivateCruise& aq);
	virtual bool Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(uint iClientID, XActivateThrusters& aq);
	virtual bool Send_FLPACKET_COMMON_SETTARGET(uint iClientID, XSetTarget& st);
	virtual void unknown_6(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual bool Send_FLPACKET_COMMON_GOTRADELANE(uint iClientID, XGoTradelane& tl);
	virtual bool Send_FLPACKET_COMMON_STOPTRADELANE(uint iClientID, uint iShip, uint iArchTradelane1, uint iArchTradelane2);
	virtual bool Send_FLPACKET_COMMON_JETTISONCARGO(uint iClientID, XJettisonCargo& jc);
	virtual bool SendPacket(void*, void*);
	virtual bool Startup(uint, uint);
	virtual void nullsub(uint);
	virtual bool Send_FLPACKET_SERVER_LOGINRESPONSE(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual bool Send_FLPACKET_SERVER_CHARACTERINFO(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual bool Send_FLPACKET_SERVER_CHARSELECTVERIFIED(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual void Shutdown();
	virtual bool DispatchMsgs();
	virtual bool CDPClientProxy__Disconnect(uint iClientID);
	virtual uint CDPClientProxy__GetSendQSize(uint iClientID);
	virtual uint CDPClientProxy__GetSendQBytes(uint iClientID);
	virtual double CDPClientProxy__GetLinkSaturation(uint iClientID);
	virtual bool Send_FLPACKET_SERVER_SETSHIPARCH(uint iClientID, uint iShipArch);
	virtual bool Send_FLPACKET_SERVER_SETHULLSTATUS(uint iClientID, float fStatus);
	virtual bool Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual bool Send_FLPACKET_SERVER_SETEQUIPMENT(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual void unknown_26(uint iClientID, uint iDunno);
	virtual bool Send_FLPACKET_SERVER_SETADDITEM(uint iClientID, FLPACKET_UNKNOWN& pDunno, FLPACKET_UNKNOWN& pDunno2);
	virtual void unknown_28(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3);
	virtual bool Send_FLPACKET_SERVER_SETSTARTROOM(uint iClientID, uint iDunno, uint iDunno2);
	virtual bool Send_FLPACKET_SERVER_GFDESTROYCHARACTER(uint iClientID, uint iDunno, uint iDunno2);
	virtual bool Send_FLPACKET_SERVER_GFUPDATECHAR(uint iClientID, uint iDunno, uint iDunno2);
	virtual bool Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(uint iClientID, uint iDunno);
	virtual bool Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(uint iClientID, uint iDunno, uint iDunno2);
	virtual bool Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(uint iClientID, uint iDunno, uint iDunno2);
	virtual bool Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(uint iClientID, uint iDunno);
	virtual void unknown_36(uint iClientID, uint iDunno, uint iDunno2);
	virtual void unknown_37(uint iClientID, uint iDunno, uint iDunno2);
	virtual bool Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(uint iClientID, uint iDunno);
	virtual bool Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(uint iClientID, uint iDunno, uint iDunno2);
	virtual bool Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(uint iClientID, uint iDunno, uint iDunno2);
	virtual bool Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(uint iClientID, uint iDunno);
	virtual bool Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(uint iClientID, uint iDunno, uint iDunno2);
	virtual bool Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(uint iClientID, uint iReason);
	virtual void unknown_44(uint iClientID, uint iDunno, uint iDunno2);
	virtual bool Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(uint iClientID, uint iDunno, uint iDunno2);
	virtual bool Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(uint iClientID, uint iDunno);
	virtual bool Send_FLPACKET_SERVER_CREATESOLAR(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual bool Send_FLPACKET_SERVER_CREATESHIP(uint iClientID, FLPACKET_CREATESHIP& pShip);
	virtual bool Send_FLPACKET_SERVER_CREATELOOT(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual bool Send_FLPACKET_SERVER_CREATEMINE(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual bool Send_FLPACKET_SERVER_CREATEGUIDED(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual bool Send_FLPACKET_SERVER_CREATECOUNTER(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual void unknown_53(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual void unknown_54(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3);
	virtual bool Send_FLPACKET_COMMON_UPDATEOBJECT(uint iClientID, SSPObjUpdateInfo& pUpdate);
	virtual bool Send_FLPACKET_SERVER_DESTROYOBJECT(uint iClientID, FLPACKET_DESTROYOBJECT& pDestroy);
	virtual bool Send_FLPACKET_SERVER_ACTIVATEOBJECT(uint iClientID, XActivateEquip& aq);
	virtual bool Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual bool Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual bool Send_FLPACKET_SERVER_LAND(uint iClientID, FLPACKET_LAND& pLand);
	EXPORT virtual bool Send_FLPACKET_SERVER_LAUNCH(uint iClientID, FLPACKET_LAUNCH& pLaunch);
	virtual bool Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(uint iClientID, bool bResponse, uint iShipID);
	virtual void unknown_63(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	EXPORT virtual bool Send_FLPACKET_SERVER_DAMAGEOBJECT(uint iClientID, uint iObj, float iDamage);
	virtual bool Send_FLPACKET_SERVER_ITEMTRACTORED(uint iClientID, uint iDunno);
	virtual bool Send_FLPACKET_SERVER_USE_ITEM(uint iClientID, uint iDunno);
	virtual bool Send_FLPACKET_SERVER_SETREPUTATION(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual void unknown_68(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual bool Send_FLPACKET_SERVER_SENDCOMM(uint iClientID, uint, uint, uint, uint, uint, 
		uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint, uint);
	virtual void unknown_70(uint iClientID, uint iDunno);
	virtual bool Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual void unknown_72(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual bool Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(uint iClientID, uint iDunno);
	virtual void unknown_74(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual void unknown_75(uint iClientID, uint iDunno);
	virtual bool Send_FLPACKET_SERVER_MARKOBJ(uint iClientID, uint iDunno, uint iDunno2);
	virtual void unknown_77(uint iClientID, uint iDunno);
	virtual bool Send_FLPACKET_SERVER_SETCASH(uint iClientID, uint iCash);
	virtual void unknown_79(uint iClientID, uint iDunno);
	virtual void unknown_80(uint iClientID, uint iDunno);
	virtual void unknown_81(uint iClientID, uint iDunno);
	virtual void unknown_82(uint iClientID, uint iDunno);
	virtual void unknown_83(uint iClientID, char* szDunno);
	virtual bool Send_FLPACKET_SERVER_REQUEST_RETURNED(uint iClientID, uint iShipID, uint iFlag, uint iDunno3, uint iDunno4);
	virtual void unknown_85(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual void unknown_86(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3);
	virtual bool Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(uint iClientID, uint iDunno, uint iDunno2);
	virtual bool Send_FLPACKET_SERVER_BURNFUSE(uint iClientID, FLPACKET_BURNFUSE& pBurnfuse);
	virtual void unknown_89(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual void unknown_90(uint iClientID);
	virtual void unknown_91(uint iClientID, uint iDunno);
	virtual bool Send_FLPACKET_COMMON_SET_WEAPON_GROUP(uint iClientID, unsigned char *p2, int p3);
	virtual bool Send_FLPACKET_COMMON_SET_VISITED_STATE(uint iClientID, unsigned char *p2, int p3);
	virtual bool Send_FLPACKET_COMMON_REQUEST_BEST_PATH(uint iClientID, unsigned char *p2, int p3);
	virtual bool Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(uint iClientID, unsigned char *p2, int p3);
	virtual void unknown_96(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3);
	virtual bool Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(uint iClientID, unsigned char *p2, int p3);
	virtual bool Send_FLPACKET_COMMON_SET_MISSION_LOG(uint iClientID, unsigned char *p2, int p3);
	virtual bool Send_FLPACKET_COMMON_SET_INTERFACE_STATE(uint iClientID, unsigned char *p2, int p3);
	virtual void unknown_100(uint iClientID, uint iDunno, uint iDunno2);
	virtual void unknown_101(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	virtual void unknown_102(uint iClientID, uint iDunno);
	virtual void unknown_103(uint iClientID, uint iDunno);
	virtual void unknown_104(uint iClientID, uint iDunno, uint iDunno2);
	virtual void unknown_105(uint iClientID, uint iDunno, uint iDunno2);
	virtual void unknown_106(uint iClientID, uint iDunno, uint iDunno2);
	virtual void unknown_107(uint iClientID, uint iDunno, uint iDunno2);
	virtual bool Send_FLPACKET_COMMON_PLAYER_TRADE(uint iClientID, uint iDunno);
	virtual void unknown_109(uint iClientID, uint iDunno);
	EXPORT virtual bool Send_FLPACKET_SERVER_SCANNOTIFY(uint iClientID, uint iDunno, uint iDunno2);
	EXPORT virtual bool Send_FLPACKET_SERVER_PLAYERLIST(uint iClientID, wchar_t*, uint, char);
	EXPORT virtual void unknown_112(uint iClientID, uint iDunno);
	EXPORT virtual bool Send_FLPACKET_SERVER_PLAYERLIST_2(uint iClientID);
	EXPORT virtual bool Send_FLPACKET_SERVER_MISCOBJUPDATE_6(uint iClientID, uint iDunno, uint iDunno2);
	EXPORT virtual bool Send_FLPACKET_SERVER_MISCOBJUPDATE_7(uint iClientID, uint iDunno, uint iDunno2);
	EXPORT virtual bool Send_FLPACKET_SERVER_MISCOBJUPDATE(uint iClientID, FLPACKET_UNKNOWN& pDunno);
	EXPORT virtual bool Send_FLPACKET_SERVER_MISCOBJUPDATE_2(uint iClientID, uint iDunno, uint iDunno2);
	EXPORT virtual bool Send_FLPACKET_SERVER_MISCOBJUPDATE_3(uint iClientID, uint iTargetID, uint iRank);
	EXPORT virtual bool Send_FLPACKET_SERVER_MISCOBJUPDATE_4(uint iClientID, uint iDunno, uint iDunno2);
	EXPORT virtual bool Send_FLPACKET_SERVER_MISCOBJUPDATE_5(uint iClientID, uint iDunno, uint iDunno2);
	virtual void unknown_121(uint iClientID, uint iDunno);  // formation sth?
	virtual bool Send_FLPACKET_SERVER_FORMATION_UPDATE(uint iClientID, uint iShipID, Vector& vFormationOffset);
	virtual void unknown_123(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3, uint iDunno4, uint iDunno5, uint iDunno6);
	virtual void unknown_124(uint iClientID);
	virtual void unknown_125(uint iClientID, uint iDunno);
	virtual int unknown_126(char* szUnknown);
};

IMPORT  void  ForceClientLogout(unsigned int);
IMPORT  int  GetClientStats(struct client_stats_t *,int *);
IMPORT  int  GetNumClients(void);
IMPORT  void  GetRemoteClientPort( std::vector<unsigned long> & );
IMPORT  void  SetRemoteClientPassword(unsigned short const *);
IMPORT  void  SetRemoteClientResponseData(bool,bool,int,unsigned short const *,unsigned int,char const *);
IMPORT  void  SetRemoteClientSessionName(unsigned short const *);
IMPORT  void  SetServerLogFunction(int (*)(struct ErrorCode,char const *,...));

extern "C" IMPORT HkIClientImpl Client;
extern "C" IMPORT HkIClientImpl * GetClientInterface();

#endif // _FLCOREREMOTECLIENT_H_
