#include "PCH.hpp"

#include "API/InternalApi.hpp"

#include "API/FLHook/ClientList.hpp"

Action<void, Error> InternalApi::FMsgEncodeXml(std::wstring_view xml, char* buffer, const uint size, uint& ret)
{
    XMLReader rdr;
    RenderDisplayList rdl;
    const std::wstring msg = std::format(L"<?xml version=\"1.0\" encoding=\"UTF-16\"?><RDL><PUSH/>{}<PARA/><POP/></RDL>\x000A\x000A", xml);

    if (!rdr.read_buffer(rdl, reinterpret_cast<const char*>(msg.c_str()), msg.length() * 2))
    {
        return { cpp::fail(Error::WrongXmlSyntax) };
    }

    BinaryRDLWriter writer;
    writer.write_buffer(rdl, buffer, size, ret);
    return { {} };
}

void InternalApi::FMsgSendChat(ClientId client, char* buffer, uint size)
{
    auto p4 = reinterpret_cast<uint>(buffer);
    uint p3 = size;
    uint p2 = 0x00010000;
    uint p1 = client.GetValue();

    auto unkClient = FLHook::hookClientImpl;
    auto rc = FLHook::rcSendChatMsg;

    __asm {
			push [p4]
			push [p3]
			push [p2]
			push [p1]
			mov ecx, [unkClient]
			add ecx, 4
			call [rc]
    }
}

Action<void, Error> InternalApi::SendMessage(const ClientId to, const std::wstring_view message, const ClientId from, std::wstring_view fromXml)
{
    if (from)
    {
        const auto fromName = StringUtils::ToLower(from.GetCharacterName().Unwrap());
        if (FLHookConfig::i()->userCommands.userCmdIgnore)
        {
            for (const auto& ignore : from.GetData().ignoreInfoList)
            {
                if (ignore.flags.find(L'p') != std::wstring::npos || (ignore.flags.find(L'i') && fromName.find(ignore.character)))
                {
                    return { {} };
                }
            }
        }
    }

    static std::array<char, 0xFFFF> buffer;
    std::fill_n(buffer.begin(), buffer.size(), '\0');

    const auto xml = std::format(L"{}{}", fromXml, message);

    uint ret;
    if (const auto err = FMsgEncodeXml(xml, buffer.data(), buffer.size(), ret).Raw(); err.has_error())
    {
        return { cpp::fail(err.error()) };
    }

    FMsgSendChat(to, buffer.data(), ret);
    return { {} };
}

uint InternalApi::CreateID(const std::wstring& nickname) { return ::CreateID(StringUtils::wstos(nickname).c_str()); }
