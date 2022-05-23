#include "../tempban_plugin/Main.h"
#include "FLHook.h"
#include "plugin.h"

constexpr int LossInterval = 4000;

namespace Plugins::ConData
{
	struct ConnectionData
	{
		// connection data
		std::list<uint> lstLoss;
		uint iLastLoss;
		uint iAverageLoss;
		std::list<uint> lstPing;
		uint iAveragePing;
		uint iPingFluctuation;
		uint iLastPacketsSent;
		uint iLastPacketsReceived;
		uint iLastPacketsDropped;
		uint iLags;
		std::list<uint> lstObjUpdateIntervalls;
		mstime tmLastObjUpdate;
		mstime tmLastObjTimestamp;

		// exception
		bool bException;
		std::string sExceptionReason;

		// Client ID (for when receiving data)
		uint iClientID;
	};

	struct ConnectionDataException final
	{
		uint iClientID;
		bool bException;
		std::string sReason;
	};

	// Inter plugin comms

	class ConDataCommunicator : public PluginCommunicator
	{
	  public:
		inline static const char* pluginName = "Advanced Connection Data";
		explicit ConDataCommunicator(std::string plug);

		HK_ERROR PluginCall(ReceiveException, ConnectionDataException);
		HK_ERROR PluginCall(ReceiveData, ConnectionData);
	};

	//! The struct that holds client info for this plugin
	struct MiscClientInfo final
	{
		bool bLightsOn = false;
		bool bShieldsDown = false;
	};

	typedef void (*TimerFunc)();

	struct Timer
	{
		TimerFunc proc;
		mstime tmIntervallMS;
		mstime tmLastCall;
	};

	struct Config final : Reflectable
	{
		std::string File() override { return "config/misc_commands.json"; }
		uint pingKick = 0;
		uint pingKickFrame = 120;
		uint fluctKick = 0;
		uint lossKick = 0;
		uint lossKickFrame = 120;
		uint lagKick = 0;
		uint lagDetectionFrame = 50;
		uint lagDetectionMin = 50;
		uint kickThreshold = 0;
		bool allowPing = true;
	};

	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;

		// Other fields
		ReturnCode returncode = ReturnCode::Default;

		ConnectionData connections[MAX_CLIENT_ID + 1];

		ConDataCommunicator* communicator = nullptr;
		TempBanCommunicator* tempBanCommunicator = nullptr;

		std::vector<Timer> timers;
	};
}; // namespace Plugins::ConData

REFL_AUTO(type(Plugins::ConData::Config), field(pingKick), field(pingKickFrame), field(fluctKick), field(lossKick), field(lossKickFrame), field(lagKick),
    field(lagDetectionFrame), field(lagDetectionMin), field(kickThreshold), field(allowPing))