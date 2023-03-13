#include <FLHook.hpp>

struct TempBanInfo
{
	std::wstring accountId;
	mstime banStart = 0;
	mstime banEnd = 0;
};

class DLL TempBanManager : public Singleton<TempBanManager>
{
private:
	std::vector<TempBanInfo> tempBanList;

public:
	void ClearFinishedTempBans();
	void AddTempBan(ClientId client, uint durationInMin, const std::wstring& reason);
	void AddTempBan(ClientId client, uint durationInMin);
	bool CheckIfTempBanned(ClientId client);
};
