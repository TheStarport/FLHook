#pragma once
//TODO: Reimplement DLL loader and De-loader.
namespace Hk::Chat
{
	DLL cpp::result<void, Error> Msg(const std::variant<uint, std::wstring>& player, const std::wstring& message);
	DLL cpp::result<void, Error> MsgS(const std::variant<std::wstring, uint>& system, const std::wstring& message);
	DLL cpp::result<void, Error> MsgU(const std::wstring& message);
	DLL cpp::result<void, Error> FMsgEncodeXML(const std::wstring& xml, char* buf, uint size, uint& ret);
	DLL cpp::result<void, Error> FMsg(ClientId client, const std::wstring& xml);
	DLL cpp::result<void, Error> FMsg(const std::variant<uint, std::wstring>& player, const std::wstring& XML);
	DLL cpp::result<void, Error> FMsgS(const std::variant<std::wstring, uint>& system, const std::wstring& XML);
	DLL cpp::result<void, Error> FMsgU(const std::wstring& xml);
	DLL std::wstring FormatMsg(MessageColor color, MessageFormat format, const std::wstring& msg);
	DLL std::wstring GetWStringFromIdS(uint idS);
	DLL cpp::result<void, Error> FormatSendChat(uint toClientId, const std::wstring& sender, const std::wstring& text, const std::wstring& textColor);
	DLL void SendGroupChat(uint fromClientId, const std::wstring& text);
	DLL void SendLocalSystemChat(uint fromClientId, const std::wstring& text);
	DLL cpp::result<void, Error> SendPrivateChat(uint fromClientId, uint toClientId, const std::wstring& text);
	DLL void SendSystemChat(uint fromClientId, const std::wstring& text);
}