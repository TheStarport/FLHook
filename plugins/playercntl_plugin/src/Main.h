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
bool extern set_bEnableMe;
bool extern set_bEnableDo;
bool extern set_bEnableWardrobe;
bool extern set_bEnableRestartCost;
bool extern set_bLocalTime;

// Imports from freelancer libraries.
namespace pub {
namespace SpaceObj {
IMPORT int __cdecl DrainShields(unsigned int);
IMPORT int __cdecl SetInvincible(unsigned int, bool, bool, float);
} // namespace SpaceObj

namespace Player {
IMPORT int GetRank(unsigned int const &iClientID, int &iRank);
}
} // namespace pub

// From EquipmentUtilities.cpp
namespace EquipmentUtilities {
void ReadIniNicknames();
const char *FindNickname(unsigned int hash);
} // namespace EquipmentUtilities

// From PurchaseRestrictions
namespace PurchaseRestrictions {
void LoadSettings(const std::string &scPluginCfgFile);
}

namespace Rename {
void LoadSettings(const std::string &scPluginCfgFile);
bool CreateNewCharacter(struct SCreateCharacterInfo const &si,
                        unsigned int iClientID);
void CharacterSelect_AFTER(struct CHARACTER_ID const &charId,
                           unsigned int iClientID);
bool UserCmd_RenameMe(uint iClientID, const std::wstring &wscCmd,
                      const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_SetMoveCharCode(uint iClientID, const std::wstring &wscCmd,
                             const std::wstring &wscParam,
                             const wchar_t *usage);
bool UserCmd_MoveChar(uint iClientID, const std::wstring &wscCmd,
                      const std::wstring &wscParam, const wchar_t *usage);
void AdminCmd_SetAccMoveCode(CCmds *cmds, const std::wstring &wscCharname,
                             const std::wstring &wscCode);
void AdminCmd_ShowTags(CCmds *cmds);
void AdminCmd_AddTag(CCmds *cmds, const std::wstring &wscTag,
                     const std::wstring &wscPassword,
                     const std::wstring &description);
void AdminCmd_DropTag(CCmds *cmds, const std::wstring &wscTag);
bool UserCmd_DropTag(uint iClientID, const std::wstring &wscCmd,
                     const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_MakeTag(uint iClientID, const std::wstring &wscCmd,
                     const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_SetTagPass(uint iClientID, const std::wstring &wscCmd,
                        const std::wstring &wscParam, const wchar_t *usage);
void Timer();
} // namespace Rename

namespace MiscCmds {
void LoadSettings(const std::string &scPluginCfgFile);
void ClearClientInfo(uint iClientID);
void BaseEnter(unsigned int iBaseID, unsigned int iClientID);
void CharacterInfoReq(unsigned int iClientID, bool p2);
void Timer();
bool UserCmd_Pos(uint iClientID, const std::wstring &wscCmd,
                 const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_Stuck(uint iClientID, const std::wstring &wscCmd,
                   const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_Dice(uint iClientID, const std::wstring &wscCmd,
                  const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_DropRep(uint iClientID, const std::wstring &wscCmd,
                     const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_Coin(uint iClientID, const std::wstring &wscCmd,
                  const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_Lights(uint iClientID, const std::wstring &wscCmd,
                    const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_SelfDestruct(uint iClientID, const std::wstring &wscCmd,
                          const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_Shields(uint iClientID, const std::wstring &wscCmd,
                     const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_Screenshot(uint iClientID, const std::wstring &wscCmd,
                        const std::wstring &wscParam, const wchar_t *usage);
void AdminCmd_SmiteAll(CCmds *cmds);
} // namespace MiscCmds

namespace IPBans {
void LoadSettings(const std::string &scPluginCfgFile);
void PlayerLaunch(unsigned int iShip, unsigned int iClientID);
void BaseEnter(unsigned int iBaseID, unsigned int iClientID);
void AdminCmd_ReloadBans(CCmds *cmds);
void AdminCmd_AuthenticateChar(CCmds *cmds, const std::wstring &wscCharname);
void ClearClientInfo(uint iClientID);
} // namespace IPBans

namespace PurchaseRestrictions {
void LoadSettings(const std::string &scPluginCfgFile);
void ClearClientInfo(unsigned int iClientID);
void PlayerLaunch(unsigned int iShip, unsigned int iClientID);
void BaseEnter(unsigned int iBaseID, unsigned int iClientID);
bool GFGoodBuy(struct SGFGoodBuyInfo const &gbi, unsigned int iClientID);
bool ReqAddItem(unsigned int goodID, char const *hardpoint, int count,
                float status, bool mounted, uint iClientID);
bool ReqChangeCash(int iMoneyDiff, unsigned int iClientID);
bool ReqEquipment(class EquipDescList const &eqDesc, unsigned int iClientID);
bool ReqHullStatus(float fStatus, unsigned int iClientID);
bool ReqSetCash(int iMoney, unsigned int iClientID);
bool ReqShipArch(unsigned int iArchID, unsigned int iClientID);
} // namespace PurchaseRestrictions

namespace HyperJump {
void LoadSettings(const std::string &scPluginCfgFile);
void Timer();
bool SystemSwitchOutComplete(unsigned int iShip, unsigned int iClientID);
void SendDeathMsg(const std::wstring &wscMsg, uint iSystem,
                  uint iClientIDVictim, uint iClientIDKiller);
void ClearClientInfo(unsigned int iClientID);
void PlayerLaunch(unsigned int iShip, unsigned int iClientID);
void MissileTorpHit(uint iClientID, DamageList *dmg);
void AdminCmd_Chase(CCmds *cmds, const std::wstring &wscCharname);
bool AdminCmd_Beam(CCmds *cmds, const std::wstring &wscCharname,
                   const std::wstring &wscTargetBaseName);
void AdminCmd_Pull(CCmds *cmds, const std::wstring &wscCharname);
void AdminCmd_Move(CCmds *cmds, float x, float y, float z);
void AdminCmd_TestBot(CCmds *cmds, const std::wstring &wscSystemNick,
                      int iCheckZoneTime);
void AdminCmd_JumpTest(CCmds *cmds, const std::wstring &fuse);

bool UserCmd_Survey(uint iClientID, const std::wstring &wscCmd,
                    const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_SetCoords(uint iClientID, const std::wstring &wscCmd,
                       const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_ChargeJumpDrive(uint iClientID, const std::wstring &wscCmd,
                             const std::wstring &wscParam,
                             const wchar_t *usage);
bool UserCmd_ActivateJumpDrive(uint iClientID, const std::wstring &wscCmd,
                               const std::wstring &wscParam,
                               const wchar_t *usage);
} // namespace HyperJump

namespace PimpShip {
void LoadSettings(const std::string &scPluginCfgFile);
void LocationEnter(unsigned int locationID, unsigned int iClientID);
bool UserCmd_PimpShip(uint iClientID, const std::wstring &wscCmd,
                      const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_ShowSetup(uint iClientID, const std::wstring &wscCmd,
                       const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_ShowItems(uint iClientID, const std::wstring &wscCmd,
                       const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_ChangeItem(uint iClientID, const std::wstring &wscCmd,
                        const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_BuyNow(uint iClientID, const std::wstring &wscCmd,
                    const std::wstring &wscParam, const wchar_t *usage);
} // namespace PimpShip

namespace CargoDrop {
void LoadSettings(const std::string &scPluginCfgFile);
void Timer();
void SendDeathMsg(const std::wstring &wscMsg, uint iSystem,
                  uint iClientIDVictim, uint iClientIDKiller);
void ClearClientInfo(unsigned int iClientID);
void SPObjUpdate(struct SSPObjUpdateInfo const &ui, unsigned int iClientID);
} // namespace CargoDrop

namespace Restart {
void LoadSettings(const std::string &scPluginCfgFile);
bool UserCmd_ShowRestarts(uint iClientID, const std::wstring &wscCmd,
                          const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_Restart(uint iClientID, const std::wstring &wscCmd,
                     const std::wstring &wscParam, const wchar_t *usage);
void Timer();
} // namespace Restart

namespace RepFixer {
void LoadSettings(const std::string &scPluginCfgFile);
void PlayerLaunch(unsigned int iShip, unsigned int iClientID);
void BaseEnter(unsigned int iBaseID, unsigned int iClientID);
} // namespace RepFixer

namespace GiveCash {
void LoadSettings(const std::string &scPluginCfgFile);
void PlayerLaunch(unsigned int iShip, unsigned int iClientID);
void BaseEnter(unsigned int iBaseID, unsigned int iClientID);
bool UserCmd_GiveCash(uint iClientID, const std::wstring &wscCmd,
                      const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_ShowCash(uint iClientID, const std::wstring &wscCmd,
                      const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_SetCashCode(uint iClientID, const std::wstring &wscCmd,
                         const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_DrawCash(uint iClientID, const std::wstring &wscCmd,
                      const std::wstring &wscParam, const wchar_t *usage);
} // namespace GiveCash

namespace Message {
void LoadSettings(const std::string &scPluginCfgFile);
void ClearClientInfo(unsigned int iClientID);
void Timer();
void DisConnect(uint iClientID, enum EFLConnection p2);
void CharacterInfoReq(unsigned int iClientID, bool p2);
void PlayerLaunch(unsigned int iShip, unsigned int iClientID);
void BaseEnter(unsigned int iBaseID, unsigned int iClientID);
void SetTarget(uint uClientID, struct XSetTarget const &p2);
bool SubmitChat(CHAT_ID cId, unsigned long p1, const void *rdl, CHAT_ID cIdTo,
                int p2);
bool HkCb_SendChat(uint iClientID, uint iTo, uint iSize, void *pRDL);
bool UserCmd_SetMsg(uint iClientID, const std::wstring &wscCmd,
                    const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_ShowMsgs(uint iClientID, const std::wstring &wscCmd,
                      const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_SMsg(uint iClientID, const std::wstring &wscCmd,
                  const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_LMsg(uint iClientID, const std::wstring &wscCmd,
                  const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_GMsg(uint iClientID, const std::wstring &wscCmd,
                  const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_ReplyToLastPMSender(uint iClientID, const std::wstring &wscCmd,
                                 const std::wstring &wscParam,
                                 const wchar_t *usage);
bool UserCmd_SendToLastTarget(uint iClientID, const std::wstring &wscCmd,
                              const std::wstring &wscParam,
                              const wchar_t *usage);
bool UserCmd_ShowLastPMSender(uint iClientID, const std::wstring &wscCmd,
                              const std::wstring &wscParam,
                              const wchar_t *usage);
bool UserCmd_PrivateMsg(uint iClientID, const std::wstring &wscCmd,
                        const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_PrivateMsgID(uint iClientID, const std::wstring &wscCmd,
                          const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_FactionMsg(uint iClientID, const std::wstring &wscCmd,
                        const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_FactionInvite(uint iClientID, const std::wstring &wscCmd,
                           const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_SetChatTime(uint iClientID, const std::wstring &wscCmd,
                         const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_Time(uint iClientID, const std::wstring &wscCmd,
                  const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_CustomHelp(uint iClientID, const std::wstring &wscCmd,
                        const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_BuiltInCmdHelp(uint iClientID, const std::wstring &wscCmd,
                            const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_MailShow(uint iClientID, const std::wstring &wscCmd,
                      const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_MailDel(uint iClientID, const std::wstring &wscCmd,
                     const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_Me(uint iClientID, const std::wstring &wscCmd,
                const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_Do(uint iClientID, const std::wstring &wscCmd,
                const std::wstring &wscParam, const wchar_t *usage);
bool RedText(std::wstring wscXMLMsg, uint iSystemID);
void UserCmd_Process(uint iClientID, const std::wstring &wscCmd);
void AdminCmd_SendMail(CCmds *cmds, const std::wstring &wscCharname,
                       const std::wstring &wscMsg);
void SendDeathMsg(const std::wstring &wscMsg, uint iSystem,
                  uint iClientIDVictim, uint iClientIDKiller);
} // namespace Message

namespace PlayerInfo {
bool UserCmd_ShowInfo(uint iClientID, const std::wstring &wscCmd,
                      const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_SetInfo(uint iClientID, const std::wstring &wscCmd,
                     const std::wstring &wscParam, const wchar_t *usage);
} // namespace PlayerInfo

namespace AntiJumpDisconnect {
void ClearClientInfo(uint iClientID);
void DisConnect(unsigned int iClientID, enum EFLConnection state);
void JumpInComplete(unsigned int iSystem, unsigned int iShip,
                    unsigned int iClientID);
void SystemSwitchOutComplete(unsigned int iShip, unsigned int iClientID);
void CharacterInfoReq(unsigned int iClientID, bool p2);
} // namespace AntiJumpDisconnect

namespace SystemSensor {
void LoadSettings(const std::string &scPluginCfgFile);
bool UserCmd_ShowScan(uint iClientID, const std::wstring &wscCmd,
                      const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_Net(uint iClientID, const std::wstring &wscCmd,
                 const std::wstring &wscParam, const wchar_t *usage);
void ClearClientInfo(uint iClientID);
void PlayerLaunch(unsigned int iShip, unsigned int iClientID);
void JumpInComplete(unsigned int iSystem, unsigned int iShip,
                    unsigned int iClientID);
void GoTradelane(unsigned int iClientID, struct XGoTradelane const &xgt);
void StopTradelane(unsigned int iClientID, unsigned int p1, unsigned int p2,
                   unsigned int p3);
void Dock_Call(unsigned int const &iShip, unsigned int const &iDockTarget,
               int iCancel, enum DOCK_HOST_RESPONSE response);
} // namespace SystemSensor

namespace Wardrobe {
void LoadSettings(const std::string &scPluginCfgFile);
bool UserCmd_ShowWardrobe(uint iClientID, const std::wstring &wscCmd,
                          const std::wstring &wscParam, const wchar_t *usage);
bool UserCmd_ChangeCostume(uint iClientID, const std::wstring &wscCmd,
                           const std::wstring &wscParam, const wchar_t *usage);
void Timer();
} // namespace Wardrobe

#endif
