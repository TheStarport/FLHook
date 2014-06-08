// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.


#ifndef __MAIN_H__
#define __MAIN_H__ 1

#include <FLHook.h>

int extern set_iPluginDebug;
bool extern set_bEnablePimpShip;
bool extern set_bEnableRenameMe;
bool extern set_bEnableMoveChar;
bool extern set_bLocalTime;

// Imports from freelancer libraries.
namespace pub
{
	namespace SpaceObj
	{
		IMPORT int __cdecl DrainShields(unsigned int);
		IMPORT int __cdecl SetInvincible(unsigned int,bool,bool,float);
	}

	namespace Player
	{
		IMPORT int GetRank(unsigned int const &iClientID, int &iRank);
	}
}

// From EquipmentUtilities.cpp
namespace EquipmentUtilities
{
	void ReadIniNicknames();
	const char *FindNickname(unsigned int hash);
}

// From PurchaseRestrictions
namespace PurchaseRestrictions
{
	void LoadSettings(const string &scPluginCfgFile);
}

namespace Rename
{
	void LoadSettings(const string &scPluginCfgFile);
	bool CreateNewCharacter(struct SCreateCharacterInfo const &si, unsigned int iClientID);
	void CharacterSelect_AFTER(struct CHARACTER_ID const &charId, unsigned int iClientID);
	bool UserCmd_RenameMe(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_SetMoveCharCode(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_MoveChar(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	void AdminCmd_SetAccMoveCode(CCmds* cmds, const wstring &wscCharname, const wstring &wscCode);
	void AdminCmd_ShowTags(CCmds* cmds);
	void AdminCmd_AddTag(CCmds* cmds, const wstring &wscTag, const wstring &wscPassword, const wstring &description);
	void AdminCmd_DropTag(CCmds* cmds, const wstring &wscTag);

	bool UserCmd_DropTag(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_MakeTag(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_SetTagPass(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);

	void Timer();
}

namespace MiscCmds
{
	void LoadSettings(const string &scPluginCfgFile);
	void ClearClientInfo(uint iClientID);
	void BaseEnter(unsigned int iBaseID, unsigned int iClientID);
	void CharacterInfoReq(unsigned int iClientID, bool p2);
	void Timer();

	bool UserCmd_Pos(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_Stuck(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_Dice(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_DropRep(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_Coin(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);

	bool UserCmd_Lights(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_SelfDestruct(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_Shields(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_Screenshot(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);

	void AdminCmd_SmiteAll(CCmds* cmds);
}

namespace IPBans
{
	void LoadSettings(const string &scPluginCfgFile);
	void PlayerLaunch(unsigned int iShip, unsigned int iClientID);
	void BaseEnter(unsigned int iBaseID, unsigned int iClientID);
	void AdminCmd_ReloadBans(CCmds* cmds);
	void AdminCmd_AuthenticateChar(CCmds* cmds, const wstring &wscCharname);
	void ClearClientInfo(uint iClientID);
}

namespace PurchaseRestrictions
{
	void LoadSettings(const string &scPluginCfgFile);
	void ClearClientInfo(unsigned int iClientID);
	void PlayerLaunch(unsigned int iShip, unsigned int iClientID);
	void BaseEnter(unsigned int iBaseID, unsigned int iClientID);
	bool GFGoodBuy(struct SGFGoodBuyInfo const &gbi, unsigned int iClientID);
	bool ReqAddItem(unsigned int goodID, char const *hardpoint, int count, float status, bool mounted, uint iClientID);
	bool ReqChangeCash(int iMoneyDiff,unsigned int iClientID);
	bool ReqEquipment(class EquipDescList const &eqDesc, unsigned int iClientID);
	bool ReqHullStatus(float fStatus, unsigned int iClientID);
	bool ReqSetCash(int iMoney,unsigned int iClientID);
	bool ReqShipArch(unsigned int iArchID, unsigned int iClientID);
}

namespace HyperJump
{
	void LoadSettings(const string &scPluginCfgFile);
	void Timer();
	bool SystemSwitchOutComplete(unsigned int iShip, unsigned int iClientID);
	void SendDeathMsg(const wstring &wscMsg, uint iSystem, uint iClientIDVictim, uint iClientIDKiller);
	void ClearClientInfo(unsigned int iClientID);
	void PlayerLaunch(unsigned int iShip, unsigned int iClientID);
	void MissileTorpHit(uint iClientID, DamageList *dmg);
	
	void AdminCmd_Chase(CCmds* cmds, const wstring &wscCharname);
	bool AdminCmd_Beam(CCmds* cmds, const wstring &wscCharname, const wstring &wscTargetBaseName);
	void AdminCmd_Pull(CCmds* cmds, const wstring &wscCharname);
	void AdminCmd_Move(CCmds* cmds, float x, float y, float z);
	void AdminCmd_TestBot(CCmds* cmds, const wstring &wscSystemNick, int iCheckZoneTime);
	void AdminCmd_JumpTest(CCmds* cmds, const wstring &fuse);

	bool UserCmd_Survey(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_SetCoords(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_ChargeJumpDrive(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_ActivateJumpDrive(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
}

namespace PimpShip
{
	void LoadSettings(const string &scPluginCfgFile);
	void LocationEnter(unsigned int locationID, unsigned int iClientID);

	bool UserCmd_PimpShip(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_ShowSetup(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_ShowItems(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_ChangeItem(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_BuyNow(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
}

namespace CargoDrop
{
	void LoadSettings(const string &scPluginCfgFile);
	void Timer();
	void SendDeathMsg(const wstring & wscMsg, uint iSystem, uint iClientIDVictim, uint iClientIDKiller);
	void ClearClientInfo(unsigned int iClientID);
	void SPObjUpdate(struct SSPObjUpdateInfo const &ui, unsigned int iClientID);
}

namespace Restart
{
	void LoadSettings(const string &scPluginCfgFile);
	bool UserCmd_ShowRestarts(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_Restart(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	void Timer();
}

namespace RepFixer
{
	void LoadSettings(const string &scPluginCfgFile);
	void PlayerLaunch(unsigned int iShip, unsigned int iClientID);
	void BaseEnter(unsigned int iBaseID, unsigned int iClientID);
}

namespace GiveCash
{
	void LoadSettings(const string &scPluginCfgFile);
	void PlayerLaunch(unsigned int iShip, unsigned int iClientID);
	void BaseEnter(unsigned int iBaseID, unsigned int iClientID);

	bool UserCmd_GiveCash(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_ShowCash(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_SetCashCode(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_DrawCash(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
}

namespace Message
{
	void LoadSettings(const string &scPluginCfgFile);
	void ClearClientInfo(unsigned int iClientID);
	void Timer();
	void DisConnect(uint iClientID, enum EFLConnection p2);
	void CharacterInfoReq(unsigned int iClientID, bool p2);
	void PlayerLaunch(unsigned int iShip, unsigned int iClientID);
	void BaseEnter(unsigned int iBaseID, unsigned int iClientID);
	void SetTarget(uint uClientID, struct XSetTarget const &p2);
	bool SubmitChat(CHAT_ID cId, unsigned long p1, const void *rdl, CHAT_ID cIdTo, int p2);
	bool HkCb_SendChat(uint iClientID, uint iTo, uint iSize, void *pRDL);

	bool UserCmd_SetMsg(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_ShowMsgs(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_SMsg(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_LMsg(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_GMsg(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_ReplyToLastPMSender(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_SendToLastTarget(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_ShowLastPMSender(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_PrivateMsg(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_PrivateMsgID(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_FactionMsg(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_FactionInvite(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_SetChatTime(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_Time(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_CustomHelp(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_BuiltInCmdHelp(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_MailShow(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_MailDel(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	void UserCmd_Process(uint iClientID, const wstring &wscCmd);

	void AdminCmd_SendMail(CCmds *cmds, const wstring &wscCharname, const wstring &wscMsg);

	void SendDeathMsg(const wstring &wscMsg, uint iSystem, uint iClientIDVictim, uint iClientIDKiller);
}

namespace PlayerInfo
{
	bool UserCmd_ShowInfo(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_SetInfo(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
}

namespace AntiJumpDisconnect
{
	void ClearClientInfo(uint iClientID);
	void DisConnect(unsigned int iClientID, enum  EFLConnection state);
	void JumpInComplete(unsigned int iSystem, unsigned int iShip, unsigned int iClientID);
	void SystemSwitchOutComplete(unsigned int iShip, unsigned int iClientID);
	void CharacterInfoReq(unsigned int iClientID, bool p2);
}


namespace SystemSensor
{
	void LoadSettings(const string &scPluginCfgFile);
	bool UserCmd_ShowScan(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	bool UserCmd_Net(uint iClientID, const wstring &wscCmd, const wstring &wscParam, const wchar_t *usage);
	void ClearClientInfo(uint iClientID);
	void PlayerLaunch(unsigned int iShip, unsigned int iClientID);
	void JumpInComplete(unsigned int iSystem, unsigned int iShip, unsigned int iClientID);
	void GoTradelane(unsigned int iClientID, struct XGoTradelane const &xgt);
	void StopTradelane(unsigned int iClientID, unsigned int p1, unsigned int p2, unsigned int p3);
	void Dock_Call(unsigned int const &iShip, unsigned int const &iDockTarget, int iCancel, enum DOCK_HOST_RESPONSE response);
}

#endif