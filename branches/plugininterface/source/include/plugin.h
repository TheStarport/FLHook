enum PLUGIN_RETURNCODE
{
	DEFAULT_RETURNCODE = 0,
	SKIPPLUGINS = 1,
	SKIPPLUGINS_NOFUNCTIONCALL = 2,
	NOFUNCTIONCALL = 3,
};

enum PLUGIN_MESSAGE;

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
	PLUGIN_HkIClientImpl_Send_FLPACKET_SERVER_LAUNCH,
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
	PLUGIN_CALLBACKS_AMOUNT,
};

__declspec(dllexport) void Plugin_Communication(PLUGIN_MESSAGE msgtype, void* msg);