#include "Global.hpp"

struct TempBanInfo
{
	std::wstring accountId;
	mstime banStart;
	mstime banEnd;
};

class TempBanManager
	//: public Singleton<TempBanManager>
{
	public:
		static std::vector<TempBanInfo> tempBanList;
};