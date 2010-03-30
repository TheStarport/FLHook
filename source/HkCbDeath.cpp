#include "hook.h"

/**************************************************************************************************************
**************************************************************************************************************/

wstring SetSizeToSmall(wstring wscDataFormat)
{
	uint iFormat = wcstoul(wscDataFormat.c_str() + 2, 0, 16);
	wchar_t wszStyleSmall[32];
	wcscpy(wszStyleSmall, wscDataFormat.c_str());
	swprintf(wszStyleSmall + wcslen(wszStyleSmall) - 2, L"%02X", 0x90 | (iFormat & 7));
	return wszStyleSmall;
}

/**************************************************************************************************************
Send "Death: ..." chat-message
**************************************************************************************************************/

void SendDeathMsg(wstring wscMsg, uint iSystemID, uint iClientIDVictim, uint iClientIDKiller)
{
	CALL_PLUGINS(PLUGIN_SendDeathMsg,(wscMsg,iSystemID,iClientIDVictim,iClientIDKiller));
	if(bPluginReturn)
		return;

	// encode xml string(default and small)
	// non-sys
	wstring wscXMLMsg = L"<TRA data=\"" + set_wscDeathMsgStyle + L"\" mask=\"-1\"/> <TEXT>";
	wscXMLMsg += XMLText(wscMsg);
	wscXMLMsg += L"</TEXT>";

	char szBuf[0xFFFF];
	uint iRet;
	if(!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsg, szBuf, sizeof(szBuf), iRet)))
		return;

	wstring wscStyleSmall = SetSizeToSmall(set_wscDeathMsgStyle);
	wstring wscXMLMsgSmall = wstring(L"<TRA data=\"") + wscStyleSmall + L"\" mask=\"-1\"/> <TEXT>";
	wscXMLMsgSmall += XMLText(wscMsg);
	wscXMLMsgSmall += L"</TEXT>";
	char szBufSmall[0xFFFF];
	uint iRetSmall;
	if(!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsgSmall, szBufSmall, sizeof(szBufSmall), iRetSmall)))
		return;

	// sys
	wstring wscXMLMsgSys = L"<TRA data=\"" + set_wscDeathMsgStyleSys + L"\" mask=\"-1\"/> <TEXT>";
	wscXMLMsgSys += XMLText(wscMsg);
	wscXMLMsgSys += L"</TEXT>";
	char szBufSys[0xFFFF];
	uint iRetSys;
	if(!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsgSys, szBufSys, sizeof(szBufSys), iRetSys)))
		return;

	wstring wscStyleSmallSys = SetSizeToSmall(set_wscDeathMsgStyleSys);
	wstring wscXMLMsgSmallSys = L"<TRA data=\"" + wscStyleSmallSys + L"\" mask=\"-1\"/> <TEXT>";
	wscXMLMsgSmallSys += XMLText(wscMsg);
	wscXMLMsgSmallSys += L"</TEXT>";
	char szBufSmallSys[0xFFFF];
	uint iRetSmallSys;
	if(!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsgSmallSys, szBufSmallSys, sizeof(szBufSmallSys), iRetSmallSys)))
		return;

	// send
	// for all players
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iClientID = HkGetClientIdFromPD(pPD);
		uint iClientSystemID = 0;
		pub::Player::GetSystem(iClientID, iClientSystemID);

		char *szXMLBuf;
		int iXMLBufRet;
		char *szXMLBufSys;
		int iXMLBufRetSys;
		if(set_bUserCmdSetDieMsgSize && (ClientInfo[iClientID].dieMsgSize == CS_SMALL)) {
			szXMLBuf = szBufSmall;
			iXMLBufRet = iRetSmall;
			szXMLBufSys = szBufSmallSys;
			iXMLBufRetSys = iRetSmallSys;
		} else {
			szXMLBuf = szBuf;
			iXMLBufRet = iRet;
			szXMLBufSys = szBufSys;
			iXMLBufRetSys = iRetSys;
		}

		if(!set_bUserCmdSetDieMsg)
		{ // /set diemsg disabled, thus send to all
			if(iSystemID == iClientSystemID)
				HkFMsgSendChat(iClientID, szXMLBufSys, iXMLBufRetSys);
			else
				HkFMsgSendChat(iClientID, szXMLBuf, iXMLBufRet);
			continue;
		}

		if(ClientInfo[iClientID].dieMsg == DIEMSG_NONE)
			continue;
		else if((ClientInfo[iClientID].dieMsg == DIEMSG_SYSTEM) && (iSystemID == iClientSystemID))  
			HkFMsgSendChat(iClientID, szXMLBufSys, iXMLBufRetSys);
		else if((ClientInfo[iClientID].dieMsg == DIEMSG_SELF) && ((iClientID == iClientIDVictim) || (iClientID == iClientIDKiller)))
			HkFMsgSendChat(iClientID, szXMLBufSys, iXMLBufRetSys);
		else if(ClientInfo[iClientID].dieMsg == DIEMSG_ALL) {
			if(iSystemID == iClientSystemID)
				HkFMsgSendChat(iClientID, szXMLBufSys, iXMLBufRetSys);
			else
				HkFMsgSendChat(iClientID, szXMLBuf, iXMLBufRet);
		}
	}
}

/**************************************************************************************************************
Called when ship was destroyed
**************************************************************************************************************/

void __stdcall ShipDestroyed(DamageList *_dmg, char *szECX, uint iKill)
{

	CALL_PLUGINS(PLUGIN_ShipDestroyed,(_dmg,szECX,iKill));
	if(bPluginReturn)
		return;

	try {
		DamageList dmg;
		try { dmg = *_dmg; } catch(...) { return; }
		// get client id
		char *szP;
		memcpy(&szP, szECX + 0x10, 4);
		uint iClientID;
		memcpy(&iClientID, szP + 0xB4, 4);

		if(iClientID && iKill) { // a player was killed
			wstring wscEvent;
			wscEvent.reserve(256);
			wscEvent = L"kill";

			uint iSystemID;
			pub::Player::GetSystem(iClientID, iSystemID);
			wchar_t wszSystem[64];
			swprintf(wszSystem, L"%u", iSystemID);

			if(!dmg.get_cause())
				dmg = ClientInfo[iClientID].dmgLast;

			uint iCause = dmg.get_cause();
			uint iClientIDKiller = HkGetClientIDByShip(dmg.get_inflictor_id());

			wstring wscVictim = (wchar_t*)Players.GetActiveCharacterName(iClientID);
			wscEvent += L" victim=" + wscVictim;
			if(iClientIDKiller) {
				wstring wscType = L"";
				if(iCause == 0x05)
					wscType = L"Missile/Torpedo";
				else if(iCause == 0x07)
					wscType = L"Mine";
				else if((iCause == 0x06) || (iCause == 0xC0) || (iCause == 0x15))
					wscType = L"Wasp/Hornet";
				else if(iCause == 0x01)
					wscType = L"Collision";
				else if(iCause == 0x02)
					wscType = L"Gun";
				else {
					wscType = L"Gun"; //0x02
//					AddLog("get_cause() returned %X", iCause);
				}

				wstring wscMsg;
				if(iClientID == iClientIDKiller) {
					wscEvent += L" type=selfkill";
					wscMsg = ReplaceStr(set_wscDeathMsgTextSelfKill, L"%victim", wscVictim);
				} else {
					wscEvent += L" type=player";
					wstring wscKiller = (wchar_t*)Players.GetActiveCharacterName(iClientIDKiller);
					wscEvent += L" by=" + wscKiller;

					wscMsg = ReplaceStr(set_wscDeathMsgTextPlayerKill, L"%victim", wscVictim);
					wscMsg = ReplaceStr(wscMsg, L"%killer", wscKiller);
				}

				wscMsg = ReplaceStr(wscMsg, L"%type", wscType);
				if(set_bDieMsg && wscMsg.length())
					SendDeathMsg(wscMsg, iSystemID, iClientID, iClientIDKiller);
				ProcessEvent(L"%s", wscEvent.c_str());

				// MultiKillMessages
				if((set_MKM_bActivated) && (iClientID != iClientIDKiller))
				{
					wstring wscKiller = (wchar_t*)Players.GetActiveCharacterName(iClientIDKiller);

					ClientInfo[iClientIDKiller].iKillsInARow++;
					foreach(set_MKM_lstMessages, MULTIKILLMESSAGE, it)
					{
						if(it->iKillsInARow == ClientInfo[iClientIDKiller].iKillsInARow)
						{
							wstring wscXMLMsg = L"<TRA data=\"" + set_MKM_wscStyle + L"\" mask=\"-1\"/> <TEXT>";
							wscXMLMsg += XMLText(ReplaceStr(it->wscMessage, L"%player", wscKiller));
							wscXMLMsg += L"</TEXT>";
							
							char szBuf[0xFFFF];
							uint iRet;
							if(!HKHKSUCCESS(HkFMsgEncodeXML(wscXMLMsg, szBuf, sizeof(szBuf), iRet)))
								break;

							// for all players in system...
							struct PlayerData *pPD = 0;
							while(pPD = Players.traverse_active(pPD))
							{
								uint iClientID = HkGetClientIdFromPD(pPD);
								uint iClientSystemID = 0;
								pub::Player::GetSystem(iClientID, iClientSystemID);
								if((iClientID == iClientIDKiller) || ((iSystemID == iClientSystemID) && (((ClientInfo[iClientID].dieMsg == DIEMSG_ALL) || (ClientInfo[iClientID].dieMsg == DIEMSG_SYSTEM)) || !set_bUserCmdSetDieMsg)))
									HkFMsgSendChat(iClientID, szBuf, iRet);
							}
						}
					}
				}
			} else if(dmg.get_inflictor_id()) {
				wstring wscType = L"";
				if(iCause == 0x05)
					wscType = L"Missile/Torpedo";
				else if(iCause == 0x07)
					wscType = L"Mine";
				else if((iCause == 0x06) || (iCause == 0xC0) || (iCause == 0x15))
					wscType = L"Wasp/Hornet";
				else if(iCause == 0x01)
					wscType = L"Collision";
				else
					wscType = L"Gun"; //0x02

				wscEvent += L" type=npc";
				wstring wscMsg = ReplaceStr(set_wscDeathMsgTextNPC, L"%victim", wscVictim);
				wscMsg = ReplaceStr(wscMsg, L"%type", wscType);

				if(set_bDieMsg && wscMsg.length())
					SendDeathMsg(wscMsg, iSystemID, iClientID, 0);
				ProcessEvent(L"%s", wscEvent.c_str());
			} else if(iCause == 0x08) {
				wscEvent += L" type=suicide";
				wstring wscMsg = ReplaceStr(set_wscDeathMsgTextSuicide, L"%victim", wscVictim);

				if(set_bDieMsg && wscMsg.length())
					SendDeathMsg(wscMsg, iSystemID, iClientID, 0);
				ProcessEvent(L"%s", wscEvent.c_str());
			} else if(iCause == 0x18) {
				wstring wscMsg = ReplaceStr(set_wscDeathMsgTextAdminKill, L"%victim", wscVictim);

				if(set_bDieMsg && wscMsg.length())
					SendDeathMsg(wscMsg, iSystemID, iClientID, 0);
			} else {
				wstring wscMsg = L"Death: " + wscVictim + L" has died";
				if(set_bDieMsg && wscMsg.length())
					SendDeathMsg(wscMsg, iSystemID, iClientID, 0);
			}
		} 

		if(iClientID)
		{
			ClientInfo[iClientID].iShipOld = ClientInfo[iClientID].iShip;
			ClientInfo[iClientID].iShip = 0;
		}
	} catch(...) { AddLog("Exception in %s", __FUNCTION__); }
}

FARPROC fpOldShipDestroyed;

__declspec(naked) void ShipDestroyedHook()
{
	__asm 
	{
		mov eax, [esp+0Ch] ; +4
		mov edx, [esp+4]
		push ecx
		push edx
		push ecx
		push eax
		call ShipDestroyed
		pop ecx
		mov eax, [fpOldShipDestroyed]
		jmp eax
	}
}
/**************************************************************************************************************
Called when base was destroyed
**************************************************************************************************************/

void BaseDestroyed(uint iObject, uint iClientIDBy)
{

	CALL_PLUGINS(PLUGIN_BaseDestroyed,(iObject,iClientIDBy));
	if(bPluginReturn)
		return;

	uint iID;
	pub::SpaceObj::GetDockingTarget(iObject, iID);
	Universe::IBase *base = Universe::get_base(iID);

	char *szBaseName = "";
	if(base)
	{
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
	}

	ProcessEvent(L"basedestroy basename=%s basehash=%u solarhash=%u by=%s",
					stows(szBaseName).c_str(),
					iObject,
					iID,
					(wchar_t*)Players.GetActiveCharacterName(iClientIDBy));

}
