#include "hook.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT bool g_bMsg = false;

HK_ERROR HkMsg(int iClientID, wstring wscMessage)
{
	struct CHAT_ID ci = {0};
	struct CHAT_ID ciClient = {iClientID};

	wstring wscXML = L"<TRA data=\"0x19BD3A00\" mask=\"-1\"/><TEXT>" + XMLText(wscMessage) + L"</TEXT>";
	uint iRet;
	char szBuf[1024];
	HkFMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet);
	g_bMsg = true;
	HkIServerImpl::SubmitChat(ci, iRet, szBuf, ciClient, -1);
	g_bMsg = false;

	return HKE_OK;
}

HK_ERROR HkMsg(wstring wscCharname, wstring wscMessage)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	return HkMsg(iClientID, wscMessage);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool g_bMsgS = false;

HK_ERROR HkMsgS(wstring wscSystemname, wstring wscMessage)
{
	uint iSystemID = 0;
	if(!(iSystemID = _wtoi(wscSystemname.c_str())))
	{
		pub::GetSystemID(iSystemID, wstos(wscSystemname).c_str());
		if(!iSystemID)
			return HKE_INVALID_SYSTEM;;
	}

	// prepare xml
	wstring wscXML = L"<TRA data=\"0xE6C68400\" mask=\"-1\"/><TEXT>" + XMLText(wscMessage) + L"</TEXT>";
	uint iRet;
	char szBuf[1024];
	HkFMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet);

	struct CHAT_ID ci = {0};

	// for all players in system...
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iClientID = HkGetClientIdFromPD(pPD);
		uint iClientSystemID = 0;
		pub::Player::GetSystem(iClientID, iClientSystemID);
		if(iSystemID == iClientSystemID)
		{
			struct CHAT_ID ciClient = {iClientID};
			g_bMsgS = true;
			HkIServerImpl::SubmitChat(ci, iRet, szBuf, ciClient, -1);
			g_bMsgS = false;
		}
	}

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool g_bMsgU = false;

HK_ERROR HkMsgU(wstring wscMessage)
{
	struct CHAT_ID ci = {0};
	struct CHAT_ID ciClient = {0x00010000};

	wstring wscXML = L"<TRA font=\"1\" color=\"#FFFFFF\"/><TEXT>" + XMLText(wscMessage) + L"</TEXT>";
	uint iRet;
	char szBuf[1024];
	HkFMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet);
	g_bMsgU = true;
	HkIServerImpl::SubmitChat(ci, iRet, szBuf, ciClient, -1);
	g_bMsgU = false;

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkFMsgEncodeXML(wstring wscXML, char *szBuf, uint iSize, uint &iRet)
{
	XMLReader rdr;
	RenderDisplayList rdl;
	wstring wscMsg = L"<?xml version=\"1.0\" encoding=\"UTF-16\"?><RDL><PUSH/>";
	wscMsg += wscXML;
	wscMsg += L"<PARA/><POP/></RDL>\x000A\x000A";
	if(!rdr.read_buffer(rdl, (const char*)wscMsg.c_str(), (uint)wscMsg.length() * 2))
		return HKE_WRONG_XML_SYNTAX;;

	BinaryRDLWriter	rdlwrite;
	rdlwrite.write_buffer(rdl, szBuf, iSize, iRet);

	return HKE_OK;;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

_RCSendChatMsg RCSendChatMsg;

HK_ERROR HkFMsgSendChat(uint iClientID, char *szBuf, uint iSize)
{
	uint p4 = (uint)szBuf;
	uint p3 = iSize;
	uint p2 = 0x00010000;
	uint p1 = iClientID;

	__asm
	{
		push [p4]
		push [p3]
		push [p2]
		push [p1]
		mov ecx, [Client]
		add ecx, 4
		call [RCSendChatMsg]
	}

	return HKE_OK;;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkFMsg(uint iClientID, wstring wscXML)
{
	char szBuf[0xFFFF];
	uint iRet;
	if(!HKHKSUCCESS(HkFMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet)))
		return HKE_WRONG_XML_SYNTAX;;

	HkFMsgSendChat(iClientID, szBuf, iRet);
	return HKE_OK;
}

HK_ERROR HkFMsg(wstring wscCharname, wstring wscXML)
{
	HK_GET_CLIENTID(iClientID, wscCharname);

	if(iClientID == -1)
		return HKE_PLAYER_NOT_LOGGED_IN;

	return HkFMsg(iClientID, wscXML);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkFMsgS(wstring wscSystemname, wstring wscXML)
{
	// get system id
	uint iSystemID = 0;
	if(!(iSystemID = _wtoi(wscSystemname.c_str())))
	{
		pub::GetSystemID(iSystemID, wstos(wscSystemname).c_str());
		if(!iSystemID)
			return HKE_INVALID_SYSTEM;;
	}
	
	// encode xml string
	char szBuf[0xFFFF];
	uint iRet;
	if(!HKHKSUCCESS(HkFMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet)))
		return HKE_WRONG_XML_SYNTAX;;


	// for all players in system...
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iClientID = HkGetClientIdFromPD(pPD);
		uint iClientSystemID = 0;
		pub::Player::GetSystem(iClientID, iClientSystemID);
		if(iSystemID == iClientSystemID)
			HkFMsgSendChat(iClientID, szBuf, iRet);
	}

	return HKE_OK;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

HK_ERROR HkFMsgU(wstring wscXML)
{
	// encode xml string
	char szBuf[0xFFFF];
	uint iRet;
	if(!HKHKSUCCESS(HkFMsgEncodeXML(wscXML, szBuf, sizeof(szBuf), iRet)))
		return HKE_WRONG_XML_SYNTAX;;

	// for all players
	struct PlayerData *pPD = 0;
	while(pPD = Players.traverse_active(pPD))
	{
		uint iClientID = HkGetClientIdFromPD(pPD);
		HkFMsgSendChat(iClientID, szBuf, iRet);
	}

	return HKE_OK;
}