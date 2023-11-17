#pragma once

class DLL TempBanManager
{
        struct TempBanInfo
        {
                std::wstring accountId;
                mstime banStart = 0;
                mstime banEnd = 0;
        };

        std::vector<TempBanInfo> tempBanList;

    public:
        void ClearFinishedTempBans();
        void AddTempBan(AccountId account, uint durationInDays);

        [[nodiscard]]
        bool CheckIfTempBanned(AccountId account) const;
};
