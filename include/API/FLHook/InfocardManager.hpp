#pragma once

class FLHook;
class InfocardManager
{
        std::map<uint, std::wstring> infocardOverride;
        std::vector<HMODULE> loadedDlls;

        InfocardManager();
        ~InfocardManager();

    public:
        InfocardManager(const InfocardManager&) = delete;
        InfocardManager& operator=(InfocardManager) = delete;
        InfocardManager(InfocardManager&&) = delete;
        InfocardManager& operator=(InfocardManager&&) = delete;

        std::wstring_view GetInfocard(uint ids) const;

        /**
         * \brief Allows you to override the specified infocard number with a new string. This functionality will be limited to server-side only without a
         * client hook. \param ids The infocard/name number that you wish to replace, this does not technically need to exist already. \param override The
         * string that you would like to replace it with. \param client An optional client id to only override an infocard for one client in particular.
         * Otherwise the change will be sent to all connected clients.
         */
        void OverrideInfocard(uint ids, const std::wstring& override, ClientId client = ClientId());
};
