#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/InternalApi.hpp"
#include "Core/IEngineHook.hpp"

std::wstring SetSizeToSmall(const std::wstring& dataFormat) { return dataFormat.substr(0, 8) + L"90"; }

void IEngineHook::SendDeathMessage(const std::wstring& msg, SystemId systemId, ClientId clientVictim, ClientId clientKiller)
{
    CallPlugins(&Plugin::OnSendDeathMessage, clientKiller, clientVictim, systemId, std::wstring_view(msg));

    // encode xml std::wstring(default and small) non-system
    const auto xmlMsg =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", FLHook::GetConfig().chatConfig.msgStyle.deathMsgStyle, StringUtils::XmlText(msg));

    static std::array<char, 0xFFFF> xmlBuf;
    uint ret;
    if (InternalApi::FMsgEncodeXml(xmlMsg, xmlBuf.data(), xmlBuf.size(), ret).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSmall = std::format(
        L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", SetSizeToSmall(FLHook::GetConfig().chatConfig.msgStyle.deathMsgStyle), StringUtils::XmlText(msg));

    static std::array<char, 0xFFFF> bufSmall;
    uint retSmall;
    if (InternalApi::FMsgEncodeXml(xmlMsgSmall, bufSmall.data(), bufSmall.size(), retSmall).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSystem =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}</TEXT>", FLHook::GetConfig().chatConfig.msgStyle.deathMsgStyleSys, StringUtils::XmlText(msg));

    static std::array<char, 0xFFFF> bufSys;
    uint retSys;
    if (InternalApi::FMsgEncodeXml(xmlMsgSystem, bufSys.data(), bufSys.size(), retSys).Raw().has_error())
    {
        return;
    }

    const auto xmlMsgSmallSys =
        std::format(L"<TRA data=\"{}\" mask=\"-1\"/> <TEXT>{}", FLHook::GetConfig().chatConfig.msgStyle.deathMsgStyleSys, StringUtils::XmlText(msg));

    static std::array<char, 0xFFFF> BufSmallSys;
    uint retSmallSys;
    if (InternalApi::FMsgEncodeXml(xmlMsgSmallSys, BufSmallSys.data(), BufSmallSys.size(), retSmallSys).Raw().has_error())
    {
        return;
    }

    for (auto& client : FLHook::Clients())
    {
        const auto system = client.id.GetSystemId().Unwrap();

        const auto& data = client.id.GetData();

        char* sendXmlBuf;
        int sendXmlRet;
        char* sendXmlBufSys;
        int sendXmlSysRet;
        if (FLHook::GetConfig().userCommands.userCmdSetDieMsgSize && data.dieMsgSize == ChatSize::Small)
        {
            sendXmlBuf = bufSmall.data();
            sendXmlRet = retSmall;
            sendXmlBufSys = BufSmallSys.data();
            sendXmlSysRet = retSmallSys;
        }
        else
        {
            sendXmlBuf = xmlBuf.data();
            sendXmlRet = ret;
            sendXmlBufSys = bufSys.data();
            sendXmlSysRet = retSys;
        }

        if (!FLHook::GetConfig().userCommands.userCmdSetDieMsg)
        {
            // /set diemsg disabled, thus send to all
            if (systemId == system)
            {
                InternalApi::FMsgSendChat(client.id, sendXmlBufSys, sendXmlSysRet);
            }
            else
            {
                InternalApi::FMsgSendChat(client.id, sendXmlBuf, sendXmlRet);
            }
            continue;
        }

        if (data.dieMsg == DieMsgType::None)
        {
            continue;
        }

        if (data.dieMsg == DieMsgType::System && systemId == system)
        {
            InternalApi::FMsgSendChat(client.id, sendXmlBufSys, sendXmlSysRet);
        }
        else if (data.dieMsg == DieMsgType::Self && (client.id == clientVictim || client.id == clientKiller))
        {
            InternalApi::FMsgSendChat(client.id, sendXmlBufSys, sendXmlSysRet);
        }
        else if (data.dieMsg == DieMsgType::All)
        {
            if (systemId == system)
            {
                InternalApi::FMsgSendChat(client.id, sendXmlBufSys, sendXmlSysRet);
            }
            else
            {
                InternalApi::FMsgSendChat(client.id, sendXmlBuf, sendXmlRet);
            }
        }

        std::ranges::fill(xmlBuf, 0);
        std::ranges::fill(bufSmall, 0);
        std::ranges::fill(bufSys, 0);
        std::ranges::fill(BufSmallSys, 0);
    }

    const std::wstring formattedMsg = StringUtils::stows(BufSmallSys.data());
    CallPlugins(&Plugin::OnSendDeathMessageAfter, clientKiller, clientVictim, systemId, std::wstring_view(formattedMsg));
}

void IEngineHook::BaseDestroyed(ObjectId objectId, ClientId clientBy)
{
    CallPlugins(&Plugin::OnBaseDestroyed, clientBy, objectId);

    uint baseId;
    pub::SpaceObj::GetDockingTarget(objectId.GetValue(), baseId);
    Universe::IBase* base = Universe::get_base(baseId);

    auto baseName = "";
    if (base)
    {
        __asm {
			pushad
			mov ecx, [base]
			mov eax, [base]
			mov eax, [eax]
			call [eax+4]
			mov [baseName], eax
			popad
        }
    }
}
