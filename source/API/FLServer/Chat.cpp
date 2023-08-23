#include "PCH.hpp"

#include "Global.hpp"
#include "API/FLServer/Chat.hpp"
#include "API/FLServer/Client.hpp"
#include "API/FLServer/Player.hpp"
#include "API/FLServer/Math.hpp"
#include "Core/ClientServerInterface.hpp"


bool g_bMsg;
bool g_bMsgU;
bool g_bMsgS;

_RCSendChatMsg RCSendChatMsg;

namespace Hk::Chat
{
    Action<void, Error> Msg(const std::variant<uint, std::wstring_view>& player, std::wstring_view message)
    {
        if (message.empty())
        {
            return { {} };
        }

        ClientId client = Client::ExtractClientID(player);

        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        const CHAT_ID ci = { 0 };
        const CHAT_ID ciClient = { client };

        const std::wstring XML = std::format(L"<TRA data=\"0x19BD3A00\" mask=\"-1\"/><TEXT>{}</TEXT>", StringUtils::XmlText(message));

        uint retVal;
        char buf[1024];
        if (auto err = FMsgEncodeXml(XML, buf, sizeof buf, retVal).Raw(); err.has_error())
        {
            return { cpp::fail(err.error()) };
        }

        IServerImplHook::SubmitChat(ci, retVal, buf, ciClient, -1);
        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void, Error> MsgS(const std::variant<std::wstring_view, uint>& system, std::wstring_view message)
    {
        if (message.empty())
        {
            return { {} };
        }

        uint systemId = 0;
        if (!system.index())
        {
            const auto systemName = std::get<std::wstring_view>(system);
            pub::GetSystemID(systemId, StringUtils::wstos(std::wstring(systemName)).c_str());
        }
        else
        {
            systemId = std::get<uint>(system);
        }

        // prepare xml
        const std::wstring xml = std::format(L"<TRA data=\"0xE6C68400\" mask=\"-1\"/><TEXT>", StringUtils::XmlText(message));
       

        uint retVal;
        char buffer[1024];
        if (const auto err = FMsgEncodeXml(xml, buffer, sizeof buffer, retVal).Raw(); err.has_error())
        {
            return { cpp::fail(err.error()) };
        }

        const CHAT_ID ci = { 0 };

        // for all players in system...

        for (const auto player : Client::getAllPlayersInSystem(systemId))
        {
            const CHAT_ID ciClient = { player };
            IServerImplHook::SubmitChat(ci, retVal, buffer, ciClient, -1);
        }

        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void, Error> MsgU(std::wstring_view message)
    {
        if (message.empty())
        {
            return { {} };
        }

        const CHAT_ID ci = { 0 };
        const CHAT_ID ciClient = { 0x00010000 };

        const std::wstring xml = std::format(L"<TRA font=\"1\" color=\"#FFFFFF\"/><TEXT>", StringUtils::XmlText(message));

 
        uint retVal;
        char buf[1024];
        if (const auto err = FMsgEncodeXml(xml, buf, sizeof buf, retVal).Raw(); err.has_error())
        {
            return { cpp::fail(err.error()) };
        }

        IServerImplHook::SubmitChat(ci, retVal, buf, ciClient, -1);
        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void, Error> FMsgEncodeXml(std::wstring_view xmring, char* buffer, uint size, uint& ret)
    {
        XMLReader rdr;
        RenderDisplayList rdl;
        std::wstring msg = std::format(L"<?xml version=\"1.0\" encoding=\"UTF-16\"?><RDL><PUSH/>{}<PARA/><POP/></RDL>\x000A\x000A", xmring);

        if (!rdr.read_buffer(rdl, (const char*)msg.c_str(), msg.length() * 2))
        {
            return { cpp::fail(Error::WrongXmlSyntax) };
        }

        BinaryRDLWriter rdlwrite;
        rdlwrite.write_buffer(rdl, buffer, size, ret);
        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    void FMsgSendChat(ClientId client, char* buffer, uint size)
    {
        auto p4 = (uint)buffer;
        uint p3 = size;
        uint p2 = 0x00010000;
        uint p1 = client;

        __asm {
			push [p4]
			push [p3]
			push [p2]
			push [p1]
			mov ecx, [HookClient]
			add ecx, 4
			call [RCSendChatMsg]
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void, Error> FMsg(ClientId client, std::wstring_view xmring)
    {
        char buf[0xFFFF];
        uint ret;
        if (const auto err = FMsgEncodeXml(xmring, buf, sizeof buf, ret).Raw(); err.has_error())
        {
            return { cpp::fail(err.error()) };
        }

        FMsgSendChat(client, buf, ret);
        return { {} };
    }

    Action<void, Error> FMsg(const std::variant<uint, std::wstring_view>& player, std::wstring_view xmring)
    {
        ClientId client = Client::ExtractClientID(player);

        if (client == UINT_MAX)
        {
            return { cpp::fail(Error::PlayerNotLoggedIn) };
        }

        return FMsg(client, xmring);
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void, Error> FMsgS(const std::variant<std::wstring_view, uint>& system, std::wstring_view xmring)
    {
        uint systemId = 0;
        if (!system.index())
        {
            const auto systemName = std::get<std::wstring_view>(system);
            pub::GetSystemID(systemId, StringUtils::wstos(std::wstring(systemName)).c_str());
        }
        else
        {
            systemId = std::get<uint>(system);
        }
        // encode xml std::wstring
        char buf[0xFFFF];
        uint ret;
        if (const auto err = FMsgEncodeXml(xmring, buf, sizeof buf, ret).Raw(); err.has_error())
        {
            return { cpp::fail(err.error()) };
        }

        // for all players in system...
        for (const auto player : Client::getAllPlayersInSystem(systemId))
        {
            FMsgSendChat(player, buf, ret);
        }

        return { {} };
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    Action<void, Error> FMsgU(const std::wstring& xmring)
    {
        // encode xml std::wstring
        char buf[0xFFFF];
        uint ret;
        const auto err = FMsgEncodeXml(xmring, buf, sizeof buf, ret).Raw();
        if (err.has_error())
        {
            return { cpp::fail(err.error()) };
        }

        // for all players
        PlayerData* playerDb = nullptr;
        while ((playerDb = Players.traverse_active(playerDb)))
        {
            ClientId client = playerDb->onlineId;
            FMsgSendChat(client, buf, ret);
        }

        return { {} };
    }

    /** Format a chat std::wstring in accordance with the receiver's preferences and
    send it. Will check that the receiver accepts chatConfig from sender and
    refuses to send if necessary. */
    Action<void, Error> FormatSendChat(uint toClientId, const std::wstring& sender, const std::wstring& text, const std::wstring& textColor)
    {
#define HAS_FLAG(a, b) ((a).flags.find(b) != -1)

        if (FLHookConfig::i()->userCommands.userCmdIgnore)
        {
            for (const auto& ignore : ClientInfo[toClientId].ignoreInfoList)
            {
                if (!HAS_FLAG(ignore, L"i") && !StringUtils::ToLower(sender).compare(StringUtils::ToLower(ignore.character)))
                {
                    return { {} }; // ignored
                }
                if (HAS_FLAG(ignore, L"i") && StringUtils::ToLower(sender).find(StringUtils::ToLower(ignore.character)) != -1)
                {
                    return { {} };
                }
                // ignored
            }
        }

        uchar format;
        // adjust chatsize
        switch (ClientInfo[toClientId].chatSize)
        {
            case CS_SMALL: format = 0x90; break;
            case CS_BIG: format = 0x10; break;
            default: format = 0x00; break;
        }

        // adjust chatstyle
        switch (ClientInfo[toClientId].chatStyle)
        {
            case CST_BOLD: format += 0x01; break;
            case CST_ITALIC: format += 0x02; break;
            case CST_UNDERLINE: format += 0x04; break;
            default: format += 0x00; break;
        }

        wchar_t wFormatBuf[8];
        swprintf(wFormatBuf, _countof(wFormatBuf), L"%02X", format);
        const std::wstring TRADataFormat = wFormatBuf;
        const std::wstring TRADataSenderColor = L"FFFFFF"; // white

        const auto XML = std::format(L"<TRA data=\"0x{}{}\" mask=\"-1\"/><TEXT>{}: </TEXT><TRA data=\"0x{}{}\" mask=\"-1\"/><TEXT>{}</TEXT>",
                                     TRADataSenderColor,
                                     TRADataFormat,
                                     StringUtils::XmlText(sender),
                                     textColor,
                                     TRADataFormat,
                                     StringUtils::XmlText(text));

        if (const auto err = FMsg(toClientId, XML).Raw(); err.has_error())
        {
            return { cpp::fail(err.error()) };
        }

        return { {} };
    }

    /** Send a player to player message */
    Action<void, Error> SendPrivateChat(uint fromClientId, uint toClientId, const std::wstring& text)
    {
        const auto Sender = Client::GetCharacterNameByID(fromClientId).Raw();
        if (Sender.has_error())
        {
            Logger::i()->Log(LogLevel::Err, std::format(L"Unable to send private chat message from client {}", fromClientId));
            return { {} };
        }

        if (FLHookConfig::i()->userCommands.userCmdIgnore)
        {
            for (const auto& ignore : ClientInfo[toClientId].ignoreInfoList)
            {
                if (HAS_FLAG(ignore, L"p"))
                {
                    return { {} };
                }
            }
        }

        // Send the message to both the sender and receiver.
        auto err = FormatSendChat(toClientId, Sender.value(), text, L"19BD3A").Raw();
        if (err.has_error())
        {
            return { cpp::fail(err.error()) };
        }

        err = FormatSendChat(fromClientId, Sender.value(), text, L"19BD3A").Raw();
        if (err.has_error())
        {
            return { cpp::fail(err.error()) };
        }

        return { {} };
    }

    /** Send a player to system message */
    void SendSystemChat(uint fromClientId, const std::wstring& text)
    {
        const std::wstring Sender = (const wchar_t*)Players.GetActiveCharacterName(fromClientId);

        // Get the player's current system.
        uint systemId;
        pub::Player::GetSystem(fromClientId, systemId);

        // For all players in system...
        for (const auto player : Client::getAllPlayersInSystem(systemId))
        {
            // Send the message a player in this system.
            FormatSendChat(player, Sender, text, L"E6C684");
        }
    }

    /** Send a player to local system message */
    void SendLocalSystemChat(uint fromClientId, const std::wstring& text)
    {
        // Don't even try to send an empty message
        if (text.empty())
        {
            return;
        }

        const auto Sender = Client::GetCharacterNameByID(fromClientId).Raw();
        if (Sender.has_error())
        {
            Logger::i()->Log(LogLevel::Err, std::format(L"Unable to send local system chat message from client {}", fromClientId));
            return;
        }

        // Get the player's current system and location in the system.
        uint systemId;
        pub::Player::GetSystem(fromClientId, systemId);

        uint fromShip;
        pub::Player::GetShip(fromClientId, fromShip);

        Vector fromShipLoc;
        Matrix fromShipDir;
        pub::SpaceObj::GetLocation(fromShip, fromShipLoc, fromShipDir);

        // For all players in system...
        for (auto player : Client::getAllPlayersInSystem(systemId))
        {
            uint ship;
            pub::Player::GetShip(player, ship);

            Vector shipLoc;
            Matrix shipDir;
            pub::SpaceObj::GetLocation(ship, shipLoc, shipDir);

            // Cheat in the distance calculation. Ignore the y-axis
            // Is player within scanner range (15K) of the sending char.
            if (static_cast<float>(sqrt(pow(shipLoc.x - fromShipLoc.x, 2) + pow(shipLoc.z - fromShipLoc.z, 2))) > 14999.0f)
            {
                continue;
            }

            // Send the message a player in this system.
            FormatSendChat(player, Sender.value(), text, L"FF8F40");
        }
    }

    /** Send a player to group message */
    void SendGroupChat(uint fromClientId, const std::wstring& text)
    {
        auto Sender = (const wchar_t*)Players.GetActiveCharacterName(fromClientId);
        // Format and send the message a player in this group.
        auto Members = Hk::Player::GetGroupMembers(Sender).Raw();
        if (Members.has_error())
        {
            return;
        }

        for (const auto& gm : Members.value())
        {
            FormatSendChat(gm.client, Sender, text, L"FF7BFF");
        }
    }

    std::vector<HINSTANCE> vDLLs;

    void UnloadStringDLLs()
    {
        for (uint i = 0; i < vDLLs.size(); i++)
        {
            FreeLibrary(vDLLs[i]);
        }
        vDLLs.clear();
    }

    void LoadStringDLLs()
    {
        UnloadStringDLLs();

        HINSTANCE hDll = LoadLibraryExW(L"resources.dll", nullptr, LOAD_LIBRARY_AS_DATAFILE); // typically resources.dll
        if (hDll)
        {
            vDLLs.push_back(hDll);
        }

        INI_Reader ini;
        if (ini.open("freelancer.ini", false))
        {
            while (ini.read_header())
            {
                if (ini.is_header("Resources"))
                {
                    while (ini.read_value())
                    {
                        if (ini.is_value("DLL"))
                        {
                            hDll = LoadLibraryExA(ini.get_value_string(0), nullptr, LOAD_LIBRARY_AS_DATAFILE);
                            if (hDll)
                            {
                                vDLLs.push_back(hDll);
                            }
                        }
                    }
                }
            }
            ini.close();
        }
    }

    std::wstring GetWStringFromIdS(uint idS)
    {
        if (wchar_t buf[1024]; LoadStringW(vDLLs[idS >> 16], idS & 0xFFFF, buf, 1024))
        {
            return buf;
        }
        return L"";
    }

    std::wstring FormatMsg(MessageColor color, MessageFormat format, const std::wstring& msg)
    {
        const uint bgrColor = Math::RgbToBgr(static_cast<uint>(color));
        const std::wstring tra = Math::UintToHexString(bgrColor, 6, true) + Math::UintToHexString(static_cast<uint>(format), 2);

        return std::format(L"<TRA data=\"{}\" mask=\"-1\"/><TEXT>{}</TEXT>", tra, StringUtils::XmlText(msg));
    }
} // namespace Hk::Chat