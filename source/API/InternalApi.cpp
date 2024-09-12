#include "PCH.hpp"

#include "API/InternalApi.hpp"

#include "API/FLHook/ClientList.hpp"

Action<void> InternalApi::FMsgEncodeXml(std::wstring_view xml, char* buffer, const uint size, uint& ret)
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
    FLHook::rcSendChatMsg(static_cast<PVOID>(reinterpret_cast<PCHAR>(&Client) + 4), client.GetValue(), 0x10000, size, buffer);
}

Action<void> InternalApi::SendMessage(const ClientId to, const std::wstring_view message, const ClientId from, std::wstring_view fromXml)
{
    if (from)
    {
        const auto fromName = StringUtils::ToLower(from.GetCharacterName().Unwrap());
        for (const auto& ignore : from.GetData().ignoreInfoList)
        {
            if (ignore.flags.find(L'p') != std::wstring::npos || (ignore.flags.find(L'i') && fromName.find(ignore.character)))
            {
                return { {} };
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

void InternalApi::ToggleNpcSpawns(const bool on)
{
    // State already matches, no extra work needed
    if ((npcEnabled && on) || (!npcEnabled && !on))
    {
        return;
    }

    npcEnabled = on;

    byte jmp;
    byte cmp;
    if (on)
    {
        jmp = '\x75';
        cmp = '\xF9';
    }
    else
    {
        jmp = '\xEB';
        cmp = '\xFF';
    }

    auto address = FLHook::Offset(FLHook::BinaryType::Content, AddressList::DisableNpcSpawns1);
    MemUtils::WriteProcMem(address, &jmp, 1);

    address = FLHook::Offset(FLHook::BinaryType::Content, AddressList::DisableNpcSpawns2);
    MemUtils::WriteProcMem(address, &cmp, 1);
}

bool InternalApi::NpcsEnabled()
{
    return npcEnabled;
}
