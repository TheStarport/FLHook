#pragma once
// TODO: Reimplement DLL loader and De-loader
namespace Hk::Chat
{
    DLL Action<void, Error> Msg(const std::variant<uint, std::wstring_view>& player, std::wstring_view message);
    DLL Action<void, Error> MsgS(const std::variant<std::wstring_view, uint>& system, std::wstring_view message);
    DLL Action<void, Error> MsgU(std::wstring_view message);
    DLL Action<void, Error> FMsgEncodeXml(std::wstring_view xml, char* buf, uint size, uint& ret);
    DLL Action<void, Error> FMsg(ClientId client, std::wstring_view xml);
    DLL Action<void, Error> FMsg(const std::variant<uint, std::wstring_view>& player, std::wstring_view XML);
    DLL Action<void, Error> FMsgS(const std::variant<std::wstring_view, uint>& system, std::wstring_view XML);
    DLL Action<void, Error> FMsgU(const std::wstring& xml);
    DLL std::wstring FormatMsg(MessageColor color, MessageFormat format, const std::wstring& msg);
    DLL Action<void, Error> FormatSendChat(uint toClientId, const std::wstring& sender, const std::wstring& text, const std::wstring& textColor);
    DLL void SendGroupChat(uint fromClientId, const std::wstring& text);
    DLL void SendLocalSystemChat(uint fromClientId, const std::wstring& text);
    DLL Action<void, Error> SendPrivateChat(uint fromClientId, uint toClientId, const std::wstring& text);
    DLL void SendSystemChat(uint fromClientId, const std::wstring& text);
    DLL void FMsgSendChat(ClientId client, char* buffer, uint size);
} // namespace Hk::Chat
