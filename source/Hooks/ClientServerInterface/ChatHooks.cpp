#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/InfocardManager.hpp"
#include "API/InternalApi.hpp"
#include "API/Utils/FlufPayload.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/Commands/AdminCommandProcessor.hpp"
#include "Core/Commands/UserCommandProcessor.hpp"
#include "Exceptions/InvalidParameterException.hpp"

std::wstring ReplaceExclamationMarkWithClientId(std::wstring commandString, uint clientId)
{
    wchar_t lastChar = L'\0';
    for (auto w = commandString.begin(); w != commandString.end(); ++w)
    {
        if (lastChar == L' ' && *w == L'!' && (w + 1 == commandString.end() || *(w + 1) == L' '))
        {
            const size_t offset = std::distance(commandString.begin(), w);
            commandString.insert(offset + 1, std::to_wstring(clientId));
            commandString.erase(offset, 1);

            // The insert and erase *can* invalidate our iterator
            w = commandString.begin();
            std::advance(w, offset);
            continue;
        }

        lastChar = *w;
    }

    return commandString;
}

/*
Replace #t and #c tags with current target name and current ship location.
Return false if tags cannot be replaced.
*/
static bool ReplaceMessageTags(ClientId client, std::wstring& msg)
{
    if (msg.find(L"#t") != -1)
    {
        ShipId ourShip = client.GetShip().Unwrap();
        if (!ourShip)
        {
            (void)client.MessageErr(L"Not in space, cannot use message containing #t");
            return false;
        }
        ClientId targetClient = ourShip.GetTarget().Handle().GetPlayer().Unwrap();
        if (!targetClient)
        {
            (void)client.MessageErr(L"No player targetted, cannot use message containing #t");
            return false;
        }

        msg = StringUtils::ReplaceStr(msg, std::wstring_view(L"#t"), targetClient.GetCharacterId().Handle().GetValue());
    }

    if (msg.find(L"#c") != -1)
    {
        SystemId system = client.GetSystemId().Handle();
        auto ship = client.GetShip().Unwrap();
        if (!ship)
        {
            (void)client.MessageErr(L"Not in space, cannot use message containing #c");
            return false;
        }

        const auto [position, _] = ship.GetPositionAndOrientation().Unwrap();

        msg = StringUtils::ReplaceStr(msg, std::wstring_view(L"#c"), std::wstring_view(system.PositionToSectorCoord(position).Handle()));
    }

    return true;
}

void SendClientSavedMsg(ClientId client, char destCode, uint msgIdx)
{
    auto msg = StringUtils::stows(client.GetData().characterData->presetMsgs[msgIdx]);
    if (msg.size() == 0)
    {
        (void)client.MessageErr(L"No message defined");
        return;
    }

    if (!ReplaceMessageTags(client, msg))
    {
        return;
    }

    const auto& sender = client.GetCharacterId().Handle();
    std::vector<ClientId> recipients;
    std::wstring colour;
    ShipId ourShip;
    ClientId targetClient;
    switch (destCode)
    {
        case 't':
            ourShip = client.GetShip().Unwrap();
            if (!ourShip)
            {
                (void)client.MessageErr(L"Not in space");
                return;
            }
            targetClient = ourShip.GetTarget().Handle().GetPlayer().Unwrap();
            if (!targetClient)
            {
                (void)client.MessageErr(L"No player targetted");
                return;
            }
            recipients.emplace_back(client); // sender is a recipient of a targetted message
            recipients.emplace_back(targetClient);
            targetClient.GetData().lastPMSender = client;
            colour = L"19BD3A";
            break;
        case 'l':
            recipients = client.GetLocalClients().Unwrap();
            colour = L"FF8F40";
            break;
        case 's':
            recipients = client.GetSystemId().Handle().GetPlayersInSystem(true).Unwrap();
            colour = L"E6C684";
            break;
        case 'g':
            GroupId group = client.GetGroup().Unwrap();
            if (!group)
            {
                (void)client.MessageErr(L"Not in group");
                return;
            }
            recipients = group.GetGroupMembers().Unwrap();
            colour = L"FF7BFF";
            break;
    }
    for (ClientId recipient : recipients)
    {
        recipient.MessageCustomXml(std::format(L"<TRA data=\"0xFFFFFF00\" mask=\"-1\"/><TEXT>{}: </TEXT><TRA data=\"0x{}00\" mask=\"-1\" /><TEXT>{}</TEXT>",
                                               StringUtils::XmlText(sender.GetValue()),
                                               colour,
                                               StringUtils::XmlText(msg)));
    }
}

bool IServerImplHook::SubmitChatInner(CHAT_ID from, ulong size, char* buffer, CHAT_ID& to, int)
{
    TryHook
    {
        const auto config = FLHook::GetConfig();

        // Group join/leave commands are not parsed
        if (to.id == static_cast<uint>(SpecialChatIds::GroupEvent))
        {
            return true;
        }

        // Anything outside normal bounds is aborted to prevent crashes
        if (to.id > static_cast<uint>(SpecialChatIds::GroupEvent) ||
            (to.id > static_cast<uint>(SpecialChatIds::PlayerMax) && to.id < static_cast<uint>(SpecialChatIds::SpecialBase)))
        {
            return false;
        }

        if (from.id == 0)
        {
            chatData.characterName = ConsoleName;
        }
        else if (from.id)
        {
            chatData.characterName = ClientId(from.id).GetCharacterId().Unwrap().GetValue();
        }
        else
        {
            chatData.characterName = L"";
        }

        // extract text from rdlReader
        BinaryRDLReader rdl;
        std::wstring strBuffer;
        strBuffer.resize(size);
        uint ret1;
        rdl.extract_text_from_buffer(reinterpret_cast<unsigned short*>(strBuffer.data()), strBuffer.size(), ret1, buffer, size);
        std::erase(strBuffer, '\0');

        // if this is a message in system chat then convert it to local unless
        // explicitly overriden by the player using /s.
        if (config->chatConfig.defaultLocalChat && to.id == static_cast<uint>(SpecialChatIds::System))
        {
            to.id = static_cast<uint>(SpecialChatIds::Local);
        }

        // fix flserver commands and change chat to id so that event logging is accurate.
        bool foundCommand = false;
        if (strBuffer[0] == '/')
        {
            const std::wstring cmdString = ReplaceExclamationMarkWithClientId(strBuffer, from.id);

            std::wstring clientIdStr = std::to_wstring(from.id);
            auto* processor = UserCommandProcessor::i();
            ClientId client{ from.id };

            if (FLHook::GetConfig()->chatConfig.echoCommands)
            {
                const std::wstring xml = std::format(
                    LR"(<TRA data="{}" mask="-1"/><TEXT>{}</TEXT>)", FLHook::GetConfig()->chatConfig.msgStyle.msgEchoStyle, StringUtils::XmlText(cmdString));
                InternalApi::SendMessage(ClientId(from.id), xml, ClientId());
            }

            if (auto task = processor->ProcessCommand(client, clientIdStr, std::wstring_view(cmdString)); task.has_value())
            {
                FLHook::GetTaskScheduler()->StoreTaskHandle(std::make_shared<Task>(std::move(*task), client));

                return false;
            }

            if (strBuffer.starts_with(L"/i ") || strBuffer.starts_with(L"/j ") || strBuffer.starts_with(L"/join ") || strBuffer.starts_with(L"/echo "))
            {
                // Allow some of FLServer's built-in commands that we might otherwise suppress
                return true;
            }

            if (strBuffer.length() == 2 && strBuffer[1] >= '0' && strBuffer[1] <= '9')
            {
                SendClientSavedMsg(client, FLHook::GetConfig()->chatConfig.defaultLocalChat ? 'l' : 's', strBuffer[1] - '0');
                return false;
            }

            if (strBuffer.length() > 2)
            {
                char destCode = strBuffer[1];
                if (destCode == 'l' || destCode == 's' || destCode == 'g' || destCode == 't')
                {
                    foundCommand = true;

                    if (strBuffer[2] == L' ')
                    {
                        if (destCode == 'g')
                        {
                            to.id = static_cast<uint>(SpecialChatIds::Group);
                        }
                        else if (destCode == 's')
                        {
                            to.id = static_cast<uint>(SpecialChatIds::System);
                        }
                        else if (destCode == 'l')
                        {
                            to.id = static_cast<uint>(SpecialChatIds::Local);
                        }
                    }
                    else if (strBuffer.length() == 3 && strBuffer[2] >= '0' && strBuffer[2] <= '9')
                    {
                        SendClientSavedMsg(client, destCode, strBuffer[2] - '0');
                        return false;
                    }
                }
            }

            if (config->chatConfig.suppressInvalidCommands && !foundCommand)
            {
                return false;
            }
        }
        else if (strBuffer[0] == '.')
        {
            if (FLHook::GetConfig()->chatConfig.echoCommands)
            {
                const std::wstring xml = std::format(
                    LR"(<TRA data="{}" mask="-1"/><TEXT>{}</TEXT>)", FLHook::GetConfig()->chatConfig.msgStyle.msgEchoStyle, StringUtils::XmlText(strBuffer));
                InternalApi::SendMessage(ClientId(from.id), xml, ClientId());
            }

            const std::wstring cmdString = ReplaceExclamationMarkWithClientId(strBuffer, from.id);
            auto* processor = AdminCommandProcessor::i();
            const auto clientStr = std::to_wstring(from.id);
            auto client = ClientId(from.id);
            if (auto task = processor->ProcessCommand(client, AllowedContext::GameOnly, clientStr, cmdString); task.has_value())
            {
                FLHook::GetTaskScheduler()->StoreTaskHandle(std::make_shared<Task>(std::move(*task), client));
            }
            return false;
        }

        if (foundCommand)
        {
            // Trim the first two characters
            strBuffer.erase(0, 2);
        }

        // Check if any other custom prefixes have been added
        if (!config->general.chatSuppressList.empty())
        {
            const auto lcBuffer = StringUtils::ToLower(strBuffer);
            for (const auto& chat : config->general.chatSuppressList)
            {
                if (lcBuffer.starts_with(chat))
                {
                    return false;
                }
            }
        }

        if (to.id >= static_cast<uint>(SpecialChatIds::PlayerMin) && to.id <= static_cast<uint>(SpecialChatIds::PlayerMax))
        {
            ClientId(to.id).GetData().lastPMSender = ClientId(from.id);
        }
    }
    // clang-format off
    }
    catch (InvalidParameterException& ex)
    {
        const auto msg = ex.Msg();
        (void)ClientId(from.id).MessageErr(msg);
        return false;
    }
    catch ([[maybe_unused]] SehException& exc)
    {
        return false;
    }
    catch ([[maybe_unused]] const StopProcessingException&)
    {
        return false;
    }
    catch (const GameException& ex)
    {
        const auto msg = ex.Msg();
        if (const auto f = ClientId(from.id))
        {
            f.Message(msg);
        }

        DEBUG("{{ex}}", { "ex", ex.Msg() });
        return false;
    }
    catch ([[maybe_unused]] std::exception& exc) { return false; }
    catch (...) { {}; }

    return true;

// clang-format on
}

void __stdcall IServerImplHook::SubmitChat(CHAT_ID cidFrom, ulong size, char* rdlReader, CHAT_ID cidTo, int genArg1)
{
    TRACE("IServerImplHook::SubmitChat fromId={{fromId}} size={{size}} toId={{toId}}", { "fromId", cidFrom.id }, { "size", size }, { "toId", cidTo.id });

    if (const auto flufPayload = FlufPayload::FromPayload(rdlReader, size);
        cidTo.id == static_cast<uint>(SpecialChatIds::SpecialBase) && flufPayload.has_value())
    {
        if (flufPayload.value().header == "fluf")
        {
            ClientId(cidFrom.id).GetData().usingFlufClientHook = true;
        }
        else
        {
            CallPlugins(&Plugin::OnPayloadReceived, ClientId(cidFrom.id), *flufPayload);
        }
        return;
    }

    const auto skip = CallPlugins(&Plugin::OnSubmitChat, ClientId(cidFrom.id), size, rdlReader, ClientId(cidTo.id), genArg1);

    if (const bool innerCheck = SubmitChatInner(cidFrom, size, rdlReader, cidTo, genArg1); !innerCheck)
    {
        return;
    }
    chatData.inSubmitChat = true;
    if (!skip)
    {
        CallServerPreamble { Server.SubmitChat(cidFrom, size, rdlReader, cidTo, genArg1); }
        CallServerPostamble(true, );
    }
    chatData.inSubmitChat = false;

    CallPlugins(&Plugin::OnSubmitChatAfter, ClientId(cidFrom.id), size, rdlReader, ClientId(cidTo.id), genArg1);
}
