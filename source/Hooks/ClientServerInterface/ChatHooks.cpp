#include "PCH.hpp"

#include "Global.hpp"

#include "API/FLServer/Chat.hpp"
#include "API/FLServer/Client.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/Commands/AdminCommandProcessor.hpp"
#include "Core/Commands/UserCommandProcessor.hpp"
#include "Exceptions/InvalidParameterException.hpp"

bool IServerImplHook::SubmitChatInner(ClientId from, ulong size, const void* rdlReader, ClientId to, int)
{
    TryHook
    {
        const auto* config = FLHookConfig::i();

        // Group join/leave commands are not parsed
        if (to.GetValue() == SpecialChatIds::GroupEvent)
        {
            return true;
        }

        // Anything outside normal bounds is aborted to prevent crashes
        if (to.GetValue() > SpecialChatIds::GroupEvent || to.GetValue() > SpecialChatIds::PlayerMax && to.GetValue() < SpecialChatIds::SpecialBase)
        {
            return false;
        }

        if (from.GetValue() == 0)
        {
            chatData->characterName = L"CONSOLE";
        }
        else if (from)
        {
            chatData->characterName = from.GetCharacterName().Unwrap();
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
        if (config->chatConfig.defaultLocalChat && to == ClientId(SpecialChatIds::System))
        {
            to = ClientId(SpecialChatIds::Local);
        }

        // fix flserver commands and change chat to id so that event logging is accurate.
        bool foundCommand = false;
        if (buffer[0] == '/')
        {
            if (UserCommandProcessor::i()->ProcessCommand(from, std::wstring_view(buffer)))
            {
                if (FLHookConfig::c()->chatConfig.echoCommands)
                {
                    const std::wstring xml = L"<TRA data=\"" + FLHookConfig::c()->chatConfig.msgStyle.msgEchoStyle + L"\" mask=\"-1\"/><TEXT>" +
                                             StringUtils::XmlText(buffer) + L"</TEXT>";
                    Hk::Chat::FMsg(from, xml);
                }

                return false;
            }

            if (buffer.length() > 2 && buffer[2] == L' ')
            {
                if (buffer[1] == 'g')
                {
                    foundCommand = true;
                    to = ClientId(SpecialChatIds::Group);
                }
                else if (buffer[1] == 's')
                {
                    foundCommand = true;
                    to = ClientId(SpecialChatIds::System);
                }
                else if (buffer[1] == 'l')
                {
                    foundCommand = true;
                    to = ClientId(SpecialChatIds::Local);
                }
            }
        }
        else if (buffer[0] == '.')
        {
            if (FLHookConfig::c()->chatConfig.echoCommands)
            {
                const std::wstring XML = L"<TRA data=\"" + FLHookConfig::c()->chatConfig.msgStyle.msgEchoStyle + L"\" mask=\"-1\"/><TEXT>" +
                                         StringUtils::XmlText(buffer) + L"</TEXT>";
                Hk::Chat::FMsg(from, XML);
            }

            const auto processor = AdminCommandProcessor::i();
            processor->SetCurrentUser(from.GetCharacterName().Handle(), AdminCommandProcessor::AllowedContext::GameOnly);
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
                Hk::Chat::FMsg(from, XML);
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
    // clang-format off
    }
    catch (InvalidParameterException& ex)
    {
        const auto msg = ex.Msg();
        from.Message(msg);
        return false;
    }
// TODO: Handle seh exception
    catch ([[maybe_unused]] SehException& exc) { {}; }
    catch ([[maybe_unused]] const StopProcessingException&) {}
    catch (const GameException& ex)
    {
        FLHook::GetLogger().Log(LogLevel::Info, ex.Msg());
        {};
    }
    catch ([[maybe_unused]] std::exception& exc) { {}; }
    catch (...) { {}; }

    return true;

// clang-format on
}

void __stdcall IServerImplHook::SubmitChat(CHAT_ID cidFrom, ulong size, const void* rdlReader, CHAT_ID cidTo, int genArg1)
{
    FLHook::GetLogger().Log(LogLevel::Trace, std::format(L"SubmitChat(\n\tuint From = {}\n\tulong size = {}\n\tuint cidTo = {}", cidFrom.id, size, cidTo.id));

    const auto skip = CallPlugins(&Plugin::OnSubmitChat, cidFrom.id, size, rdlReader, cidTo.id, genArg1);

    if (const bool innerCheck = SubmitChatInner(ClientId(cidFrom.id), size, rdlReader, ClientId(cidTo.id), genArg1); !innerCheck)
    {
        return;
    }
    chatData->inSubmitChat = true;
    if (!skip)
    {
        CallServerPreamble { Server.SubmitChat(cidFrom, size, rdlReader, cidTo, genArg1); }
        CallServerPostamble(true, );
    }
    chatData->inSubmitChat = false;

    CallPlugins(&Plugin::OnSubmitChatAfter, cidFrom.id, size, rdlReader, cidTo.id, genArg1);
}
