#pragma once

/**
 * @brief
 * This API is used for manipulating the internal state of FLHook or a shared utility function that
 * requires certain game information to be used correctly.
 */
class DLL InternalApi
{
    public:
        InternalApi() = delete;

        /**
         * @brief Converts the provided wstring view into a byte buffer that can be sent to a client.
         * @param xml A valid freelancer XML string
         * @param buffer The output string buffer, controlled by the caller, suggest using std::array<char, 1024>
         * It is up to the caller to ensure that the provided buffer is entirely empty (null characters, before calling)
         * @param size The size of the output buffer
         * @param ret Return code from Freelancer's internal function. Purpose not entirely understood.
         * @returns On success : 'buffer' will now contain valid bytes.
         * @returns on fail : Error code of InvalidXml or Segfault in the event of invalid buffer sizes.
         * @note The encoding of the XML string is assumed to be UTF-16
         */
        static Action<void, Error> FMsgEncodeXml(std::wstring_view xml, char* buffer, uint size, uint& ret);


        static void FMsgSendChat(ClientId client, char* buffer, uint size);
        static Action<void, Error> SendMessage(ClientId to, std::wstring_view message, ClientId from = ClientId(), std::wstring_view = L"");

        static uint CreateID(const std::wstring& nickname);

        /**
         * @brief Toggles Whether NPCs spawned should be turned on or off. Does not apply to missions, only affects SpacePop
         */
        static void ToggleNpcSpawns(bool on);
};
