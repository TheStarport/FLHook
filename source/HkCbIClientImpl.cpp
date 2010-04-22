#include "hook.h"

#define CALL_CLIENT_METHOD(Method) \
	void* vRet; \
	char *tmp; \
	memcpy(&tmp, &Client, 4); \
	memcpy(&Client, &OldClient, 4); \
	HookClient->Method; \
	__asm { mov [vRet], eax } \
	memcpy(&Client, &tmp, 4); \


/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::CDPClientProxy__Disconnect(uint iClientID)
{

	CALL_CLIENT_METHOD(CDPClientProxy__Disconnect(iClientID));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

double HkIClientImpl::CDPClientProxy__GetLinkSaturation(uint iClientID)
{

	char *tmp; 
	WriteProcMem(&tmp, &Client, 4); 
	WriteProcMem(&Client, &OldClient, 4); 
	double dRet = HookClient->CDPClientProxy__GetLinkSaturation(iClientID); 
	WriteProcMem(&Client, &tmp, 4); 

	return dRet;
}

/**************************************************************************************************************
**************************************************************************************************************/

uint HkIClientImpl::CDPClientProxy__GetSendQBytes(uint iClientID)
{

	CALL_CLIENT_METHOD(CDPClientProxy__GetSendQBytes(iClientID));
	return reinterpret_cast<uint>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

uint HkIClientImpl::CDPClientProxy__GetSendQSize(uint iClientID)
{

	CALL_CLIENT_METHOD(CDPClientProxy__GetSendQSize(iClientID));
	return reinterpret_cast<uint>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::nullsub(uint)
{

	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATECRUISE(uint iClientID, XActivateCruise& aq)
{



	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_ACTIVATECRUISE(iClientID, aq));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATEEQUIP(unsigned int iClientID, struct XActivateEquip &aq)
{


	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_ACTIVATEEQUIP(iClientID, aq));
	return reinterpret_cast<bool>(vRet);

}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(uint iClientID, XActivateThrusters& aq)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_ACTIVATETHRUSTERS(iClientID, aq));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_FIREWEAPON(uint iClientID, FLPACKET_UNKNOWN& pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_FIREWEAPON(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_GOTRADELANE(uint iClientID, XGoTradelane& tl)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_GOTRADELANE(iClientID, tl));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_JETTISONCARGO(uint iClientID, XJettisonCargo& jc)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_JETTISONCARGO(iClientID, jc));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_PLAYER_TRADE(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_PLAYER_TRADE(iClientID, iDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_BEST_PATH(uint iClientID, unsigned char *p2, int p3)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_REQUEST_BEST_PATH(iClientID, p2, p3));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(uint iClientID, unsigned char *p2, int p3)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_REQUEST_GROUP_POSITIONS(iClientID, p2, p3));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(uint iClientID, unsigned char *p2, int p3)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_REQUEST_PLAYER_STATS(iClientID, p2, p3));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_INTERFACE_STATE(uint iClientID, unsigned char *p2, int p3)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_SET_INTERFACE_STATE(iClientID, p2, p3));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_MISSION_LOG(uint iClientID, unsigned char *p2, int p3)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_SET_MISSION_LOG(iClientID, p2, p3));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_VISITED_STATE(uint iClientID, unsigned char *p2, int p3)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_SET_VISITED_STATE(iClientID, p2, p3));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_SET_WEAPON_GROUP(uint iClientID, unsigned char *p2, int p3)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_SET_WEAPON_GROUP(iClientID, p2, p3));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_SETTARGET(uint iClientID, XSetTarget& st)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_SETTARGET(iClientID, st));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_STOPTRADELANE(uint iClientID, uint iShip, uint iArchTradelane1, uint iArchTradelane2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_STOPTRADELANE(iClientID, iShip, iArchTradelane1, iArchTradelane2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_COMMON_UPDATEOBJECT(uint iClientID, SSPObjUpdateInfo& pUpdate)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_COMMON_UPDATEOBJECT(iClientID, pUpdate));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_ACTIVATEOBJECT(uint iClientID, XActivateEquip& aq)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_ACTIVATEOBJECT(iClientID, aq));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_BURNFUSE(uint iClientID, FLPACKET_BURNFUSE& pBurnfuse)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_BURNFUSE(iClientID, pBurnfuse));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_CHARACTERINFO(uint iClientID, FLPACKET_UNKNOWN& pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CHARACTERINFO(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_CHARSELECTVERIFIED(uint iClientID, FLPACKET_UNKNOWN& pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CHARSELECTVERIFIED(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATECOUNTER(uint iClientID, FLPACKET_UNKNOWN& pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CREATECOUNTER(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATEGUIDED(uint iClientID, FLPACKET_UNKNOWN& pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CREATEGUIDED(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATELOOT(uint iClientID, FLPACKET_UNKNOWN& pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CREATELOOT(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATEMINE(uint iClientID, FLPACKET_UNKNOWN& pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CREATEMINE(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl_Send_FLPACKET_SERVER_CREATESHIP_AFTER(uint iClientID, FLPACKET_CREATESHIP& pShip)
{
	CALL_PLUGINS(PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_CREATESHIP_AFTER,(iClientID, pShip));
}

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATESHIP(uint iClientID, FLPACKET_CREATESHIP& pShip)
{

	CALL_PLUGINS(PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_CREATESHIP,(iClientID, pShip));
	if(bPluginReturn)
		return reinterpret_cast<bool>(vPluginRet);

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CREATESHIP(iClientID, pShip));

	HkIClientImpl_Send_FLPACKET_SERVER_CREATESHIP_AFTER(iClientID, pShip);

	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_CREATESOLAR(uint iClientID, FLPACKET_UNKNOWN& pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_CREATESOLAR(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_DAMAGEOBJECT(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_DAMAGEOBJECT(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_DESTROYOBJECT(uint iClientID, FLPACKET_DESTROYOBJECT& pDestroy)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_DESTROYOBJECT(iClientID, pDestroy));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_FORMATION_UPDATE(uint iClientID, uint iShipID, Vector& vFormationOffset)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_FORMATION_UPDATE(iClientID, iShipID, vFormationOffset));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFCOMPLETEAMBIENTSCRIPTLIST(iClientID, iDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFCOMPLETECHARLIST(iClientID, iDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFCOMPLETEMISSIONCOMPUTERLIST(iClientID, iDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFCOMPLETENEWSBROADCASTLIST(iClientID, iDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFCOMPLETESCRIPTBEHAVIORLIST(iClientID, iDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFDESTROYMISSIONCOMPUTER(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFDESTROYSCRIPTBEHAVIOR(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFMISSIONVENDORACCEPTANCE(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(uint iClientID, uint iReason)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFMISSIONVENDORWHYEMPTY(iClientID, iReason));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFSCRIPTBEHAVIOR(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATECHAR(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFUPDATECHAR(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFUPDATEMISSIONCOMPUTER(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFUPDATENEWSBROADCAST(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_ITEMTRACTORED(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_ITEMTRACTORED(iClientID, iDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_LAND(uint iClientID, FLPACKET_LAND& pLand)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_LAND(iClientID, pLand));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_LAUNCH(uint iClientID, FLPACKET_LAUNCH& pLaunch)
{

	CALL_PLUGINS(PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_LAUNCH,(iClientID, pLaunch));
	if(bPluginReturn)
		return reinterpret_cast<bool>(vPluginRet);

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_LAUNCH(iClientID, pLaunch));
	return reinterpret_cast<bool>(vRet);
}


/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_LOGINRESPONSE(uint iClientID, FLPACKET_UNKNOWN& pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_LOGINRESPONSE(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_MARKOBJ(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MARKOBJ(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_6(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MISCOBJUPDATE_6(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_7(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MISCOBJUPDATE_7(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE(uint iClientID, FLPACKET_UNKNOWN& pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MISCOBJUPDATE(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_2(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MISCOBJUPDATE_2(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_3(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MISCOBJUPDATE_3(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_4(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MISCOBJUPDATE_4(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_MISCOBJUPDATE_5(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_MISCOBJUPDATE_5(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_OBJECTCARGOUPDATE(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST(uint iClientID, wchar_t* wszName, uint iDunno, char szDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_PLAYERLIST(iClientID, wszName, iDunno, szDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_PLAYERLIST_2(uint iClientID)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_PLAYERLIST_2(iClientID));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_REQUEST_RETURNED(uint iClientID, uint iShipID, uint iFlag, uint iDunno3, uint iDunno4)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_REQUEST_RETURNED(iClientID, iShipID, iFlag, iDunno3, iDunno4));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(uint iClientID, bool bResponse, uint iShipID) 
{

	CALL_PLUGINS(PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP,(iClientID, bResponse, iShipID));
	if(bPluginReturn)
		return reinterpret_cast<bool>(vPluginRet);

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP(iClientID, bResponse, iShipID));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_SCANNOTIFY(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SCANNOTIFY(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_SENDCOMM(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3,
												uint iDunno4, uint iDunno5, uint iDunno6, uint iDunno7,
												uint iDunno8, uint iDunno9, uint iDunno10, uint iDunno11,
												uint iDunno12, uint iDunno13, uint iDunno14, uint iDunno15,
												uint iDunno16, uint iDunno17, uint iDunno18, uint iDunno19,
												uint iDunno20, uint iDunno21, uint iDunno22)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SENDCOMM(iClientID, iDunno, iDunno2, iDunno3, iDunno4, iDunno5, iDunno6, iDunno7, iDunno8, iDunno9, iDunno10
		, iDunno11, iDunno12, iDunno13, iDunno14, iDunno15, iDunno16, iDunno17, iDunno18, iDunno19, iDunno20, iDunno21, iDunno22));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(uint iClientID, FLPACKET_UNKNOWN& pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SET_MISSION_MESSAGE(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_SETADDITEM(uint iClientID, FLPACKET_UNKNOWN& pDunno, FLPACKET_UNKNOWN& pDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETADDITEM(iClientID, pDunno, pDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_SETCASH(uint iClientID, uint iCash)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETCASH(iClientID, iCash));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(uint iClientID, FLPACKET_UNKNOWN& pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETCOLLISIONGROUPS(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_SETEQUIPMENT(uint iClientID, FLPACKET_UNKNOWN& pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETEQUIPMENT(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_SETHULLSTATUS(uint iClientID, float fStatus)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETHULLSTATUS(iClientID, fStatus));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETMISSIONOBJECTIVES(iClientID, iDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_SETREPUTATION(uint iClientID, FLPACKET_UNKNOWN &pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETREPUTATION(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_SETSHIPARCH(uint iClientID, uint iShipArch)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETSHIPARCH(iClientID, iShipArch));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_SETSTARTROOM(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SETSTARTROOM(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_GFDESTROYCHARACTER(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_GFDESTROYCHARACTER(iClientID, iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}


/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(uint iClientID, FLPACKET_UNKNOWN &pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SYSTEM_SWITCH_IN(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(uint iClientID, FLPACKET_UNKNOWN &pDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_SYSTEM_SWITCH_OUT(iClientID, pDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Send_FLPACKET_SERVER_USE_ITEM(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(Send_FLPACKET_SERVER_USE_ITEM(iClientID, iDunno));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::SendPacket(void* pDunno1, void* pDunno2)
{

	CALL_CLIENT_METHOD(SendPacket(pDunno1, pDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::Shutdown()
{

	CALL_CLIENT_METHOD(Shutdown());
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::Startup(uint iDunno, uint iDunno2)
{

	// load universe / we load the universe directly before the server becomes internet accessible
	lstBases.clear();
	Universe::IBase *base = Universe::GetFirstBase();
	while(base)
	{
		BASE_INFO bi;
		bi.bDestroyed = false;
		bi.iObjectID = base->lSpaceObjID;
		char *szBaseName = "";
		__asm
		{
			pushad
			mov ecx, [base]
			mov eax, [base]
			mov eax, [eax]
			call [eax+4]
			mov [szBaseName], eax
			popad
		}
		
		bi.scBasename = szBaseName;
		bi.iBaseID = CreateID(szBaseName);
		lstBases.push_back(bi);
		pub::System::LoadSystem(base->iSystemID);

		base = Universe::GetNextBase();
	}

	CALL_CLIENT_METHOD(Startup(iDunno, iDunno2));
	return reinterpret_cast<bool>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

bool HkIClientImpl::DispatchMsgs()
{
		
/*	long lRet; 
	char *tmp; 
	WriteProcMem(&tmp, &Client, 4); 
	WriteProcMem(&Client, &OldClient, 4); 
	HookClient->DispatchMsgs(); 
	__asm { mov [lRet], eax } 
	WriteProcMem(&Client, &tmp, 4); */

	cdpserver->DispatchMsgs(); // calls IServerImpl functions, which also call HkIClientImpl functions
	return true;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_100(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(unknown_100(iClientID, iDunno, iDunno2));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_101(uint iClientID, FLPACKET_UNKNOWN &pDunno)
{

	CALL_CLIENT_METHOD(unknown_101(iClientID, pDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_102(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_102(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_103(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_103(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_104(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(unknown_104(iClientID, iDunno, iDunno2));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_105(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(unknown_105(iClientID, iDunno, iDunno2));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_106(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(unknown_106(iClientID, iDunno, iDunno2));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_107(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(unknown_107(iClientID, iDunno, iDunno2));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_109(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_109(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_112(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_112(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_121(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_121(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_123(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3, uint iDunno4, uint iDunno5, uint iDunno6)
{

	CALL_CLIENT_METHOD(unknown_123(iClientID, iDunno, iDunno2, iDunno3, iDunno4, iDunno5, iDunno6));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_124(uint iClientID)
{

	CALL_CLIENT_METHOD(unknown_124(iClientID));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_125(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_125(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

int HkIClientImpl::unknown_126(char *szUnknown)
{

	CALL_CLIENT_METHOD(unknown_126(szUnknown));
	return reinterpret_cast<int>(vRet);
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_26(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_26(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_28(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3)
{

	CALL_CLIENT_METHOD(unknown_28(iClientID, iDunno, iDunno2, iDunno3));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_36(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(unknown_36(iClientID, iDunno, iDunno2));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_37(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(unknown_37(iClientID, iDunno, iDunno2));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_44(uint iClientID, uint iDunno, uint iDunno2)
{

	CALL_CLIENT_METHOD(unknown_44(iClientID, iDunno, iDunno2));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_53(uint iClientID, FLPACKET_UNKNOWN &pDunno)
{

	CALL_CLIENT_METHOD(unknown_53(iClientID, pDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_54(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3)
{

	CALL_CLIENT_METHOD(unknown_54(iClientID, iDunno, iDunno2, iDunno3));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_6(uint iClientID, FLPACKET_UNKNOWN &pDunno)
{

	CALL_CLIENT_METHOD(unknown_6(iClientID, pDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_63(uint iClientID, FLPACKET_UNKNOWN &pDunno)
{

	CALL_CLIENT_METHOD(unknown_63(iClientID, pDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_68(uint iClientID, FLPACKET_UNKNOWN &pDunno)
{

	CALL_CLIENT_METHOD(unknown_68(iClientID, pDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_70(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_70(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_72(uint iClientID, FLPACKET_UNKNOWN &pDunno)
{

	CALL_CLIENT_METHOD(unknown_72(iClientID, pDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_74(uint iClientID, FLPACKET_UNKNOWN &pDunno)
{

	CALL_CLIENT_METHOD(unknown_74(iClientID, pDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_75(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_75(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_77(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_77(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_79(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_79(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_80(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_80(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_81(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_81(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_82(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_82(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_83(uint iClientID, char *szDunno)
{

	CALL_CLIENT_METHOD(unknown_83(iClientID, szDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_85(uint iClientID, FLPACKET_UNKNOWN &pDunno)
{

	CALL_CLIENT_METHOD(unknown_85(iClientID, pDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_86(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3)
{

	CALL_CLIENT_METHOD(unknown_86(iClientID, iDunno, iDunno2, iDunno3));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_89(uint iClientID, FLPACKET_UNKNOWN &pDunno)
{

	CALL_CLIENT_METHOD(unknown_89(iClientID, pDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_90(uint iClientID)
{

	CALL_CLIENT_METHOD(unknown_90(iClientID));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_91(uint iClientID, uint iDunno)
{

	CALL_CLIENT_METHOD(unknown_91(iClientID, iDunno));
	return;
}

/**************************************************************************************************************
**************************************************************************************************************/

void HkIClientImpl::unknown_96(uint iClientID, uint iDunno, uint iDunno2, uint iDunno3)
{

	CALL_CLIENT_METHOD(unknown_96(iClientID, iDunno, iDunno2, iDunno3));
	return;
}