#include "PCH.hpp"

#include "API/InternalApi.hpp"

#include "API/Utils/Logger.hpp"
#include "API/Utils/PerfTimer.hpp"
#include "Core/ClientServerInterface.hpp"
#include "Core/Commands/AdminCommandProcessor.hpp"
#include "Core/Commands/UserCommandProcessor.hpp"
#include "Exceptions/InvalidParameterException.hpp"

bool IServerImplHook::SubmitChatInner(CHAT_ID from, ulong size, const void* rdlReader, CHAT_ID& to, int)
{
    TryHook
    {
        const auto& config = FLHook::GetConfig();

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
            chatData.characterName = L"CONSOLE";
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
        std::wstring buffer;
        buffer.resize(size);
        uint ret1;
        rdl.extract_text_from_buffer((unsigned short*)buffer.data(), buffer.size(), ret1, static_cast<const char*>(rdlReader), size);
        std::erase(buffer, '\0');

        // if this is a message in system chat then convert it to local unless
        // explicitly overriden by the player using /s.
        if (config.chatConfig.defaultLocalChat && to.id == static_cast<uint>(SpecialChatIds::System))
        {
            to.id = static_cast<uint>(SpecialChatIds::Local);
        }

        // fix flserver commands and change chat to id so that event logging is accurate.
        bool foundCommand = false;
        if (buffer[0] == '/')
        {
            if (UserCommandProcessor::i()->ProcessCommand(ClientId(from.id), std::wstring_view(buffer)))
            {
                if (FLHook::GetConfig().chatConfig.echoCommands)
                {
                    const std::wstring xml = std::format(
                        LR"(<TRA data="{}" mask="-1"/><TEXT>{}</TEXT>)", FLHook::GetConfig().chatConfig.msgStyle.msgEchoStyle, StringUtils::XmlText(buffer));
                    InternalApi::SendMessage(ClientId(from.id), xml);
                }

                return false;
            }

            if (buffer.length() > 2 && buffer[2] == L' ')
            {
                if (buffer[1] == 'g')
                {
                    foundCommand = true;
                    to.id = static_cast<uint>(SpecialChatIds::Group);
                }
                else if (buffer[1] == 's')
                {
                    foundCommand = true;
                    to.id = static_cast<uint>(SpecialChatIds::System);
                }
                else if (buffer[1] == 'l')
                {
                    foundCommand = true;
                    to.id = static_cast<uint>(SpecialChatIds::Local);
                }
            }
        }
        else if (buffer[0] == '.')
        {
            if (FLHook::GetConfig().chatConfig.echoCommands)
            {
                const std::wstring xml = std::format(
                    LR"(<TRA data="{}" mask="-1"/><TEXT>{}</TEXT>)", FLHook::GetConfig().chatConfig.msgStyle.msgEchoStyle, StringUtils::XmlText(buffer));
                InternalApi::SendMessage(ClientId(from.id), xml);
            }

            const auto processor = AdminCommandProcessor::i();
            processor->SetCurrentUser(ClientId(from.id).GetCharacterName().Handle(), AdminCommandProcessor::AllowedContext::GameOnly);
            processor->ProcessCommand(std::wstring_view(buffer.begin() + 1, buffer.end()));
            return false;
        }

        // check if chat should be suppressed for in-built command prefixes
        if (buffer[0] == L'/')
        {
            if (FLHook::GetConfig().chatConfig.echoCommands)
            {
                const std::wstring xml = std::format(
                    LR"(<TRA data="{}" mask="-1"/><TEXT>{}</TEXT>)", FLHook::GetConfig().chatConfig.msgStyle.msgEchoStyle, StringUtils::XmlText(buffer));
                InternalApi::SendMessage(ClientId(from.id), xml);
            }

            if (config.chatConfig.suppressInvalidCommands && !foundCommand)
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
        if (!config.general.chatSuppressList.empty())
        {
            const auto lcBuffer = StringUtils::ToLower(buffer);
            for (const auto& chat : config.general.chatSuppressList)
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
        (void)ClientId(from.id).Message(msg);
        return false;
    }
// TODO: Handle seh exception
    catch ([[maybe_unused]] SehException& exc) { {}; }
    catch ([[maybe_unused]] const StopProcessingException&) {}
    catch (const GameException& ex)
    {
        Logger::Info(ex.Msg());
        {};
    }
    catch ([[maybe_unused]] std::exception& exc) { {}; }
    catch (...) { {}; }

    return true;

// clang-format on
}

void __stdcall IServerImplHook::SubmitChat(CHAT_ID cidFrom, ulong size, const void* rdlReader, CHAT_ID cidTo, int genArg1)
{
    Logger::Trace(std::format(L"SubmitChat(\n\tuint From = {}\n\tulong size = {}\n\tuint cidTo = {}", cidFrom.id, size, cidTo.id));

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
