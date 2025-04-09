#include "PCH.hpp"

#include "API/InternalApi.hpp"

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

bool IServerImplHook::SubmitChatInner(CHAT_ID from, ulong size, const void* rdlReader, CHAT_ID& to, int)
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
        std::wstring buffer;
        buffer.resize(size);
        uint ret1;
        rdl.extract_text_from_buffer((unsigned short*)buffer.data(), buffer.size(), ret1, static_cast<const char*>(rdlReader), size);
        std::erase(buffer, '\0');

        // if this is a message in system chat then convert it to local unless
        // explicitly overriden by the player using /s.
        if (config->chatConfig.defaultLocalChat && to.id == static_cast<uint>(SpecialChatIds::System))
        {
            to.id = static_cast<uint>(SpecialChatIds::Local);
        }

        // fix flserver commands and change chat to id so that event logging is accurate.
        bool foundCommand = false;
        if (buffer[0] == '/')
        {
            const std::wstring cmdString = ReplaceExclamationMarkWithClientId(buffer, from.id);

            std::wstring clientIdStr = std::to_wstring(from.id);
            auto processor = UserCommandProcessor::i();
            if (auto task = processor->ProcessCommand(ClientId(from.id), clientIdStr, std::wstring_view(cmdString)); task.has_value())
            {
                if (FLHook::GetConfig()->chatConfig.echoCommands)
                {
                    const std::wstring xml = std::format(LR"(<TRA data="{}" mask="-1"/><TEXT>{}</TEXT>)",
                                                         FLHook::GetConfig()->chatConfig.msgStyle.msgEchoStyle,
                                                         StringUtils::XmlText(cmdString));
                    InternalApi::SendMessage(ClientId(from.id), xml);
                }

                auto t = std::make_shared<Task>(*task);
                t->SetClient(ClientId(from.id));
                t->HandleException();

                if (t->UpdateStatus() != TaskStatus::Finished)
                {
                    FLHook::GetTaskScheduler()->AddTask(t);
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
            if (FLHook::GetConfig()->chatConfig.echoCommands)
            {
                const std::wstring xml = std::format(
                    LR"(<TRA data="{}" mask="-1"/><TEXT>{}</TEXT>)", FLHook::GetConfig()->chatConfig.msgStyle.msgEchoStyle, StringUtils::XmlText(buffer));
                InternalApi::SendMessage(ClientId(from.id), xml);
            }

            const std::wstring cmdString = ReplaceExclamationMarkWithClientId(buffer, from.id);
            const auto processor = AdminCommandProcessor::i();
            const auto clientStr = std::to_wstring(from.id);
            if (auto response = processor->ProcessCommand(ClientId(from.id), AllowedContext::GameOnly, clientStr, cmdString); response.has_value())
            {
                auto t = std::make_shared<Task>(*response);
                t->SetClient(ClientId(from.id));
                t->HandleException();

                if (t->UpdateStatus() != TaskStatus::Finished)
                {
                    FLHook::GetTaskScheduler()->AddTask(t);
                }
            }
            return false;
        }

        // check if chat should be suppressed for in-built command prefixes
        if (buffer[0] == L'/')
        {
            if (FLHook::GetConfig()->chatConfig.echoCommands)
            {
                const std::wstring xml = std::format(
                    LR"(<TRA data="{}" mask="-1"/><TEXT>{}</TEXT>)", FLHook::GetConfig()->chatConfig.msgStyle.msgEchoStyle, StringUtils::XmlText(buffer));
                InternalApi::SendMessage(ClientId(from.id), xml);
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

void __stdcall IServerImplHook::SubmitChat(CHAT_ID cidFrom, ulong size, const void* rdlReader, CHAT_ID cidTo, int genArg1)
{

    TRACE(L"{0} {1} {2}",
          { L"cidFrom", std::to_wstring(cidFrom.id) },
          { L"size", std::to_wstring(size) },
          { L"cidTo", std::to_wstring(cidTo.id) });

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
