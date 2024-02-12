#pragma once

class AbstractExternalCommandProcessor
{
    public:
        virtual ~AbstractExternalCommandProcessor() = default;
        virtual std::optional<yyjson_mut_doc*> ProcessCommand(yyjson_mut_doc* command) = 0;
        virtual std::vector<std::wstring_view> GetCommands() = 0;
};
