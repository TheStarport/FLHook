#ifndef __PLUGIN_H__
#define __PLUGIN_H__ 1

enum PLUGIN_RETURNCODE
{
	DEFAULT_RETURNCODE = 0,
	SKIPPLUGINS = 1,
	SKIPPLUGINS_NOFUNCTIONCALL = 2,
	NOFUNCTIONCALL = 3,
};



enum PLUGIN_CALLBACKS
{
	PLUGIN_HkIServerImpl_Update,
	PLUGIN_HkIServerImpl_SubmitChat,
	PLUGIN_HkIServerImpl_SubmitChat_AFTER,
	PLUGIN_HkIServerImpl_PlayerLaunch,
	PLUGIN_HkIServerImpl_PlayerLaunch_AFTER,
	PLUGIN_HkIServerImpl_FireWeapon,
	PLUGIN_HkIServerImpl_FireWeapon_AFTER,
	PLUGIN_HkIServerImpl_SPMunitionCollision,
	PLUGIN_HkIServerImpl_SPMunitionCollision_AFTER,
	PLUGIN_HkIServerImpl_SPObjUpdate,
	PLUGIN_HkIServerImpl_SPObjUpdate_AFTER,
	PLUGIN_HkIServerImpl_SPObjCollision,
	PLUGIN_HkIServerImpl_SPObjCollision_AFTER,
	PLUGIN_HkIServerImpl_LaunchComplete,
	PLUGIN_HkIServerImpl_LaunchComplete_AFTER,
	PLUGIN_HkIServerImpl_CharacterSelect,
	PLUGIN_HkIServerImpl_CharacterSelect_AFTER,
	PLUGIN_HkIServerImpl_BaseEnter,
	PLUGIN_HkIServerImpl_BaseEnter_AFTER,
	PLUGIN_HkIServerImpl_BaseExit,
	PLUGIN_HkIServerImpl_BaseExit_AFTER,
	PLUGIN_HkIServerImpl_OnConnect,
	PLUGIN_HkIServerImpl_OnConnect_AFTER,
	PLUGIN_HkIServerImpl_DisConnect,
	PLUGIN_HkIServerImpl_DisConnect_AFTER,
	PLUGIN_HkIServerImpl_TerminateTrade,
	PLUGIN_HkIServerImpl_TerminateTrade_AFTER,
	PLUGIN_HkIServerImpl_InitiateTrade,
	PLUGIN_HkIServerImpl_InitiateTrade_AFTER,
	PLUGIN_HkIServerImpl_ActivateEquip,
	PLUGIN_HkIServerImpl_ActivateEquip_AFTER,
	PLUGIN_HkIServerImpl_ActivateCruise,
	PLUGIN_HkIServerImpl_ActivateCruise_AFTER,
	PLUGIN_HkIServerImpl_ActivateThrusters,
	PLUGIN_HkIServerImpl_ActivateThrusters_AFTER,
	PLUGIN_HkIServerImpl_GFGoodSell,
	PLUGIN_HkIServerImpl_GFGoodSell_AFTER,
	PLUGIN_HkIServerImpl_CharacterInfoReq,
	PLUGIN_HkIServerImpl_CharacterInfoReq_AFTER,
	PLUGIN_HkIServerImpl_JumpInComplete,
	PLUGIN_HkIServerImpl_JumpInComplete_AFTER,
	PLUGIN_HkIServerImpl_SystemSwitchOutComplete,
	PLUGIN_HkIServerImpl_SystemSwitchOutComplete_AFTER,
	PLUGIN_HkIServerImpl_Login,
	PLUGIN_HkIServerImpl_Login_AFTER,
	PLUGIN_HkIServerImpl_MineAsteroid,
	PLUGIN_HkIServerImpl_MineAsteroid_AFTER,
	PLUGIN_HkIServerImpl_GoTradelane,
	PLUGIN_HkIServerImpl_GoTradelane_AFTER,
	PLUGIN_HkIServerImpl_StopTradelane,
	PLUGIN_HkIServerImpl_StopTradelane_AFTER,
	PLUGIN_HkIServerImpl_AbortMission,
	PLUGIN_HkIServerImpl_AbortMission_AFTER,
	PLUGIN_HkIServerImpl_AcceptTrade,
	PLUGIN_HkIServerImpl_AcceptTrade_AFTER,
	PLUGIN_HkIServerImpl_AddTradeEquip,
	PLUGIN_HkIServerImpl_AddTradeEquip_AFTER,
	PLUGIN_HkIServerImpl_BaseInfoRequest,
	PLUGIN_HkIServerImpl_BaseInfoRequest_AFTER,
	PLUGIN_HkIServerImpl_CreateNewCharacter,
	PLUGIN_HkIServerImpl_CreateNewCharacter_AFTER,
	PLUGIN_HkIServerImpl_DelTradeEquip,
	PLUGIN_HkIServerImpl_DelTradeEquip_AFTER,
	PLUGIN_HkIServerImpl_DestroyCharacter,
	PLUGIN_HkIServerImpl_DestroyCharacter_AFTER,
	PLUGIN_HkIServerImpl_GFGoodBuy,
	PLUGIN_HkIServerImpl_GFGoodBuy_AFTER,
	PLUGIN_HkIServerImpl_GFGoodVaporized,
	PLUGIN_HkIServerImpl_GFGoodVaporized_AFTER,
	PLUGIN_HkIServerImpl_GFObjSelect,
	PLUGIN_HkIServerImpl_GFObjSelect_AFTER,
	PLUGIN_HkIServerImpl_Hail,
	PLUGIN_HkIServerImpl_Hail_AFTER,
	PLUGIN_HkIServerImpl_InterfaceItemUsed,
	PLUGIN_HkIServerImpl_InterfaceItemUsed_AFTER,
	PLUGIN_HkIServerImpl_JettisonCargo,
	PLUGIN_HkIServerImpl_JettisonCargo_AFTER,
	PLUGIN_HkIServerImpl_LocationEnter,
	PLUGIN_HkIServerImpl_LocationEnter_AFTER,
	PLUGIN_HkIServerImpl_LocationExit,
	PLUGIN_HkIServerImpl_LocationExit_AFTER,
	PLUGIN_HkIServerImpl_LocationInfoRequest,
	PLUGIN_HkIServerImpl_LocationInfoRequest_AFTER,
	PLUGIN_HkIServerImpl_MissionResponse,
	PLUGIN_HkIServerImpl_MissionResponse_AFTER,
	PLUGIN_HkIServerImpl_ReqAddItem,
	PLUGIN_HkIServerImpl_ReqAddItem_AFTER,
	PLUGIN_HkIServerImpl_ReqChangeCash,
	PLUGIN_HkIServerImpl_ReqChangeCash_AFTER,
	PLUGIN_HkIServerImpl_ReqCollisionGroups,
	PLUGIN_HkIServerImpl_ReqCollisionGroups_AFTER,
	PLUGIN_HkIServerImpl_ReqEquipment,
	PLUGIN_HkIServerImpl_ReqEquipment_AFTER,
	PLUGIN_HkIServerImpl_ReqHullStatus,
	PLUGIN_HkIServerImpl_ReqHullStatus_AFTER,
	PLUGIN_HkIServerImpl_ReqModifyItem,
	PLUGIN_HkIServerImpl_ReqModifyItem_AFTER,
	PLUGIN_HkIServerImpl_ReqRemoveItem,
	PLUGIN_HkIServerImpl_ReqRemoveItem_AFTER,
	PLUGIN_HkIServerImpl_ReqSetCash,
	PLUGIN_HkIServerImpl_ReqSetCash_AFTER,
	PLUGIN_HkIServerImpl_ReqShipArch,
	PLUGIN_HkIServerImpl_ReqShipArch_AFTER,
	PLUGIN_HkIServerImpl_RequestBestPath,
	PLUGIN_HkIServerImpl_RequestBestPath_AFTER,
	PLUGIN_HkIServerImpl_RequestCancel,
	PLUGIN_HkIServerImpl_RequestCancel_AFTER,
	PLUGIN_HkIServerImpl_RequestCreateShip,
	PLUGIN_HkIServerImpl_RequestCreateShip_AFTER,
	PLUGIN_HkIServerImpl_RequestEvent,
	PLUGIN_HkIServerImpl_RequestEvent_AFTER,
	PLUGIN_HkIServerImpl_RequestGroupPositions,
	PLUGIN_HkIServerImpl_RequestGroupPositions_AFTER,
	PLUGIN_HkIServerImpl_RequestPlayerStats,
	PLUGIN_HkIServerImpl_RequestPlayerStats_AFTER,
	PLUGIN_HkIServerImpl_RequestRankLevel,
	PLUGIN_HkIServerImpl_RequestRankLevel_AFTER,
	PLUGIN_HkIServerImpl_RequestTrade,
	PLUGIN_HkIServerImpl_RequestTrade_AFTER,
	PLUGIN_HkIServerImpl_SPRequestInvincibility,
	PLUGIN_HkIServerImpl_SPRequestInvincibility_AFTER,
	PLUGIN_HkIServerImpl_SPRequestUseItem,
	PLUGIN_HkIServerImpl_SPRequestUseItem_AFTER,
	PLUGIN_HkIServerImpl_SPScanCargo,
	PLUGIN_HkIServerImpl_SPScanCargo_AFTER,
	PLUGIN_HkIServerImpl_SetInterfaceState,
	PLUGIN_HkIServerImpl_SetInterfaceState_AFTER,
	PLUGIN_HkIServerImpl_SetManeuver,
	PLUGIN_HkIServerImpl_SetManeuver_AFTER,
	PLUGIN_HkIServerImpl_SetTarget,
	PLUGIN_HkIServerImpl_SetTarget_AFTER,
	PLUGIN_HkIServerImpl_SetTradeMoney,
	PLUGIN_HkIServerImpl_SetTradeMoney_AFTER,
	PLUGIN_HkIServerImpl_SetVisitedState,
	PLUGIN_HkIServerImpl_SetVisitedState_AFTER,
	PLUGIN_HkIServerImpl_SetWeaponGroup,
	PLUGIN_HkIServerImpl_SetWeaponGroup_AFTER,
	PLUGIN_HkIServerImpl_Shutdown,
	PLUGIN_HkIServerImpl_Startup,
	PLUGIN_HkIServerImpl_Startup_AFTER,
	PLUGIN_HkIServerImpl_StopTradeRequest,
	PLUGIN_HkIServerImpl_StopTradeRequest_AFTER,
	PLUGIN_HkIServerImpl_TractorObjects,
	PLUGIN_HkIServerImpl_TractorObjects_AFTER,
	PLUGIN_HkIServerImpl_TradeResponse,
	PLUGIN_HkIServerImpl_TradeResponse_AFTER,
	PLUGIN_ClearClientInfo,
	PLUGIN_LoadUserCharSettings,
	PLUGIN_HkCb_SendChat,
	PLUGIN_HkCB_MissileTorpHit,
	PLUGIN_HkCb_AddDmgEntry,
	PLUGIN_HkCb_AddDmgEntry_AFTER,
	PLUGIN_HkCb_GeneralDmg,
	PLUGIN_AllowPlayerDamage,
	PLUGIN_SendDeathMsg,
	PLUGIN_ShipDestroyed,
	PLUGIN_BaseDestroyed,
	PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_CREATESHIP,
	PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_CREATESHIP_AFTER,
	PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_CREATESOLAR,
	PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_LAUNCH,

	PLUGIN_HkIClientImpl_Send_FLPACKET_COMMON_UPDATEOBJECT,
	PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_ACTIVATEOBJECT,
	PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_DESTROYOBJECT,
	PLUGIN_HkIClientImpl_Send_FLPACKET_COMMON_FIREWEAPON,
	PLUGIN_HkIClientImpl_Send_FLPACKET_COMMON_ACTIVATEEQUIP,
	PLUGIN_HkIClientImpl_Send_FLPACKET_COMMON_ACTIVATECRUISE,
	PLUGIN_HkIClientImpl_Send_FLPACKET_COMMON_ACTIVATETHRUSTERS,

	PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_MISCOBJUPDATE_3,
	PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_MISCOBJUPDATE_3_AFTER,
	PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_MISCOBJUPDATE_5,
	PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_REQUESTCREATESHIPRESP,
	PLUGIN_HkIEngine_CShip_init,
	PLUGIN_HkIEngine_CShip_destroy,
	PLUGIN_HkCb_Update_Time,
	PLUGIN_HkCb_Update_Time_AFTER,
	PLUGIN_HkCb_Dock_Call,
	PLUGIN_HkCb_Elapse_Time,
	PLUGIN_HkCb_Elapse_Time_AFTER,
	PLUGIN_LaunchPosHook,
	PLUGIN_HkTimerCheckKick,
	PLUGIN_HkTimerNPCAndF1Check,
	PLUGIN_UserCmd_Help,
	PLUGIN_UserCmd_Process,
	PLUGIN_CmdHelp_Callback,
	PLUGIN_ExecuteCommandString_Callback,
	PLUGIN_ProcessEvent_BEFORE,
	PLUGIN_LoadSettings,
	PLUGIN_Plugin_Communication,
	PLUGIN_CALLBACKS_AMOUNT,
};

struct PLUGIN_HOOKINFO
{
	PLUGIN_HOOKINFO(FARPROC* pFunc, PLUGIN_CALLBACKS eCallbackID, int iPriority)
	{
		this->pFunc = pFunc;
		this->eCallbackID = eCallbackID;
		this->iPriority = iPriority;
	}

	FARPROC* pFunc;
	PLUGIN_CALLBACKS eCallbackID;
	int iPriority;
};

struct PLUGIN_INFO
{
	string sName;
	string sShortName;
	bool bMayPause;
	bool bMayUnload;
	PLUGIN_RETURNCODE* ePluginReturnCode;
	list<PLUGIN_HOOKINFO> lstHooks;
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
	DSACE_SPEED_EXCEPTION = 41,
	CUSTOM_BASE_BEAM = 42,
	CUSTOM_BASE_IS_DOCKED = 43
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

struct CUSTOM_BASE_BEAM_STRUCT
{
	uint iClientID;
	uint iTargetBaseID;
	bool bBeamed;
};

struct CUSTOM_BASE_IS_DOCKED_STRUCT
{
	uint iClientID;
	uint iDockedBaseID;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif