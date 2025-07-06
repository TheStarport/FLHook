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
            chatData.characterName = ClientId(from.id).GetCharacterName().Unwrap();
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
            if (auto task = processor->ProcessCommand(client, clientIdStr, std::wstring_view(cmdString)); task.has_value())
            {
                if (FLHook::GetConfig()->chatConfig.echoCommands)
                {
                    const std::wstring xml = std::format(LR"(<TRA data="{}" mask="-1"/><TEXT>{}</TEXT>)",
                                                         FLHook::GetConfig()->chatConfig.msgStyle.msgEchoStyle,
                                                         StringUtils::XmlText(cmdString));
                    InternalApi::SendMessage(ClientId(from.id), xml, ClientId());
                }

                FLHook::GetTaskScheduler()->StoreTaskHandle(std::make_shared<Task>(std::move(*task), client));

                return false;
            }

            if (strBuffer.length() > 2 && strBuffer[2] == L' ')
            {
                if (strBuffer[1] == 'g')
                {
                    foundCommand = true;
                    to.id = static_cast<uint>(SpecialChatIds::Group);
                }
                else if (strBuffer[1] == 's')
                {
                    foundCommand = true;
                    to.id = static_cast<uint>(SpecialChatIds::System);
                }
                else if (strBuffer[1] == 'l')
                {
                    foundCommand = true;
                    to.id = static_cast<uint>(SpecialChatIds::Local);
                }
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

        // check if chat should be suppressed for in-built command prefixes
        if (strBuffer[0] == L'/')
        {
            if (FLHook::GetConfig()->chatConfig.echoCommands)
            {
                const std::wstring xml = std::format(
                    LR"(<TRA data="{}" mask="-1"/><TEXT>{}</TEXT>)", FLHook::GetConfig()->chatConfig.msgStyle.msgEchoStyle, StringUtils::XmlText(strBuffer));
                InternalApi::SendMessage(ClientId(from.id), xml, ClientId());
            }

            if (config->chatConfig.suppressInvalidCommands && !foundCommand)
            {
                return false;
            }
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
    }
    // clang-format off
    }
    catch (InvalidParameterException& ex)
    {
        const auto msg = ex.Msg();
        (void)ClientId(from.id).Message(msg);
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
        DEBUG(std::wstring(ex.Msg()));
        return false;
    }
    catch ([[maybe_unused]] std::exception& exc) { return false; }
    catch (...) { {}; }

    return true;

// clang-format on
}

void __stdcall IServerImplHook::SubmitChat(CHAT_ID cidFrom, ulong size, char* rdlReader, CHAT_ID cidTo, int genArg1)
{

    TRACE(L"{0} {1} {2}", { L"cidFrom", std::to_wstring(cidFrom.id) }, { L"size", std::to_wstring(size) }, { L"cidTo", std::to_wstring(cidTo.id) });

    if (auto flufPayload = FlufPayload::FromPayload(rdlReader, size); cidTo.id == static_cast<uint>(SpecialChatIds::SpecialBase) && flufPayload.has_value())
    {
        if (strncmp(flufPayload.value().header, "fluf", sizeof(flufPayload.value().header)) == 0)
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
