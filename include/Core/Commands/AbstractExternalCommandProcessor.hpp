#pragma once
#include "Defs/BsonWrapper.hpp"

class AbstractExternalCommandProcessor
{
    public:
        virtual ~AbstractExternalCommandProcessor() = default;
        /**
         * @brief Process an incoming command from the external message queue.
         * @param command The string key indicating which command should be executed
         * @param document A BSON document containing data for this request. It may be used and consumed as nessasary,
         * but not all commands will require extra data beyond the string identifier.
         * @return A pair of bool and shared pointer of type BsonWrapper.
         * The boolean indicates successful processing of the specified command with the provided data.
         * The shared pointer is the document that will be returned via the reply-to queue, if one has been specified.
         * Providing 'nullptr' instead of a document will prevent any response being sent, but will still confirm recepit, if the boolean is true.
         * If success is false, and the document is a 'nullptr', it is assumed that no command was not found,
         * and another command handler should attempt processing.
         */
        virtual std::pair<bool, std::shared_ptr<BsonWrapper>> ProcessCommand(std::string_view command, B_VIEW document) = 0;
        virtual std::vector<std::wstring> GetCommands() = 0;
};
