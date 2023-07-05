#include "PCH.hpp"
#include "Global.hpp"
#include "Core/Commands/AdminCommandProcessor.hpp"
#include "Core/Commands/UserCommandProcessor.hpp"
#include "Core/ClientServerInterface.hpp"

namespace IServerImplHook
{
    bool SubmitChat__Inner(CHAT_ID cidFrom, ulong size, const void* rdlReader, CHAT_ID& cidTo, int)
    {
        TRY_HOOK
        {
            const auto* config = FLHookConfig::i();

            // Group join/leave commands are not parsed
            if (cidTo.id == SpecialChatIds::GROUP_EVENT)
            {
                return true;
            }

            // Anything outside normal bounds is aborted to prevent crashes
            if (cidTo.id > SpecialChatIds::GROUP_EVENT || cidTo.id > SpecialChatIds::PLAYER_MAX && cidTo.id < SpecialChatIds::SPECIAL_BASE)
            {
                return false;
            }

            if (cidFrom.id == 0)
            {
                chatData->characterName = L"CONSOLE";
            }
            else if (Hk::Client::IsValidClientID(cidFrom.id))
            {
                chatData->characterName = Hk::Client::GetCharacterNameByID(cidFrom.id).Unwrap();
            }
            else
            {
                chatData->characterName = L"";
            }

            // extract text from rdlReader
            BinaryRDLReader rdl;
            std::wstring buffer;
            buffer.resize(size);
            uint ret1;
            rdl.extract_text_from_buffer((unsigned short*)buffer.data(), buffer.size(), ret1, static_cast<const char*>(rdlReader), size);
            std::erase(buffer, '\0');

            // if this is a message in system chat then convert it to local unless
            // explicitly overriden by the player using /s.
            if (config->chatConfig.defaultLocalChat && cidTo.id == SpecialChatIds::SYSTEM)
            {
                cidTo.id = SpecialChatIds::LOCAL;
            }

            // fix flserver commands and change chat to id so that event logging is accurate.
            bool foundCommand = false;
            if (buffer[0] == '/')
            {
                if (UserCommandProcessor::i()->ProcessCommand(cidFrom.id, std::wstring_view(buffer)))
                {
                    if (FLHookConfig::c()->chatConfig.echoCommands)
                    {
                        const std::wstring xml = L"<TRA data=\"" + FLHookConfig::c()->chatConfig.msgStyle.msgEchoStyle + L"\" mask=\"-1\"/><TEXT>" +
                                                 StringUtils::XmlText(buffer) + L"</TEXT>";
                        Hk::Chat::FMsg(cidFrom.id, xml);
                    }

                    return false;
                }

                if (buffer.length() > 2 && buffer[2] == L' ')
                {
                    if (buffer[1] == 'g')
                    {
                        foundCommand = true;
                        cidTo.id = SpecialChatIds::GROUP;
                    }
                    else if (buffer[1] == 's')
                    {
                        foundCommand = true;
                        cidTo.id = SpecialChatIds::SYSTEM;
                    }
                    else if (buffer[1] == 'l')
                    {
                        foundCommand = true;
                        cidTo.id = SpecialChatIds::LOCAL;
                    }
                }
            }
            else if (buffer[0] == '.')
            {
                if (FLHookConfig::c()->chatConfig.echoCommands)
                {
                    const std::wstring XML = L"<TRA data=\"" + FLHookConfig::c()->chatConfig.msgStyle.msgEchoStyle + L"\" mask=\"-1\"/><TEXT>" +
                                             StringUtils::XmlText(buffer) + L"</TEXT>";
                    Hk::Chat::FMsg(cidFrom.id, XML);
                }

                const auto processor = AdminCommandProcessor::i();
                processor->SetCurrentUser(Hk::Client::GetCharacterNameByID(cidFrom.id).Handle(), AdminCommandProcessor::AllowedContext::GameOnly);
                processor->ProcessCommand(std::wstring_view(buffer.begin() + 1, buffer.end()));
                return false;
            }

            // check if chat should be suppressed for in-built command prefixes
            if (buffer[0] == L'/')
            {
                if (FLHookConfig::c()->chatConfig.echoCommands)
                {
                    const std::wstring XML = L"<TRA data=\"" + FLHookConfig::c()->chatConfig.msgStyle.msgEchoStyle + L"\" mask=\"-1\"/><TEXT>" +
                                             StringUtils::XmlText(buffer) + L"</TEXT>";
                    Hk::Chat::FMsg(cidFrom.id, XML);
                }

                if (config->chatConfig.suppressInvalidCommands && !foundCommand)
                {
                    return false;
                }
            }

            if (foundCommand)
            {
                // Trim the first two characters
                buffer.erase(0, 2);
            }

            // Check if any other custom prefixes have been added
            if (!config->general.chatSuppressList.empty())
            {
                const auto lcBuffer = StringUtils::ToLower(buffer);
                for (const auto& chat : config->general.chatSuppressList)
                {
                    if (lcBuffer.rfind(chat, 0) == 0)
                    {
                        return false;
                    }
                }
            }
        }
        CATCH_HOOK({})

        return true;
    }

    void __stdcall SubmitChat(CHAT_ID cidFrom, ulong size, const void* rdlReader, CHAT_ID cidTo, int genArg1)
    {
        Logger::i()->Log(LogLevel::Trace, std::format(L"SubmitChat(\n\tuint From = {}\n\tulong size = {}\n\tuint cidTo = {}", cidFrom.id, size, cidTo.id));

        auto skip = CallPluginsBefore<void>(HookedCall::IServerImpl__SubmitChat, cidFrom.id, size, rdlReader, cidTo.id, genArg1);

        if (const bool innerCheck = SubmitChat__Inner(cidFrom, size, rdlReader, cidTo, genArg1); !innerCheck)
        {
            return;
        }
        chatData->inSubmitChat = true;
        if (!skip)
        {
            CALL_SERVER_PREAMBLE { Server.SubmitChat(cidFrom, size, rdlReader, cidTo, genArg1); }
            CALL_SERVER_POSTAMBLE(true, );
        }
        chatData->inSubmitChat = false;

        CallPluginsAfter(HookedCall::IServerImpl__SubmitChat, cidFrom.id, size, rdlReader, cidTo.id, genArg1);
    }
} // namespace IServerImplHook