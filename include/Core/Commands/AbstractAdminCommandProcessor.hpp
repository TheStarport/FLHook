#pragma once

class AbstractAdminCommandProcessor
{
    public:
        virtual ~AbstractAdminCommandProcessor() = default;
        virtual std::wstring ProcessCommand(std::wstring_view cmd, std::vector<std::wstring>& paramVector) = 0;
        virtual std::vector<std::tuple<std::wstring_view, std::wstring_view, std::wstring_view>> GetCommands() = 0;
};

template <class T>
concept IsAdminCommandProcessor = std::is_base_of_v<AbstractAdminCommandProcessor, T>;
