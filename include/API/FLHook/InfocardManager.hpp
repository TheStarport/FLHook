#pragma once

class FLHook;
class DLL InfocardManager
{
        friend FLHook;
        std::unordered_map<uint, std::wstring> infoCardOverride;
        std::unordered_map<uint, std::wstring> infoNameOverride;
        std::vector<HMODULE> loadedDlls;
        static constexpr std::string_view header = "infocard_override";

    public:
        struct InfocardPayload
        {
                std::unordered_map<uint, std::string> infoCards;
                std::unordered_map<uint, std::string> infoNames;
        };

        InfocardManager();
        ~InfocardManager();
        InfocardManager(const InfocardManager&) = delete;
        InfocardManager& operator=(InfocardManager) = delete;
        InfocardManager(InfocardManager&&) = delete;
        InfocardManager& operator=(InfocardManager&&) = delete;

        [[nodiscard]]
        std::wstring_view GetInfoName(uint ids) const;

        /**
         * \brief Allows you to override the specified infocard number with a new string.
         * This functionality will be limited to server-side only without a client hook.
         * \param ids The infocard/name number that you wish to replace, this does not technically need to exist already.
         * \param override The string that you would like to replace it with.
         * \param isName Specifies whether to update an infocard or infoname.
         * \param client An optional client id to only override an infocard for one client in particular.
         * Otherwise the change will be sent to all connected clients.
         */
        void OverrideInfocard(uint ids, const std::wstring& override, bool isName, ClientId client = {});
        void OverrideInfocards(const InfocardPayload& payload, ClientId client = {});

        void ClearOverride(uint ids, bool all = false);
};
