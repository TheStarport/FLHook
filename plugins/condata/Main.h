#pragma once

#include "../tempban/Main.h"
#include <FLHook.hpp>
#include "plugin.h"

constexpr int LossInterval = 4;

namespace Plugins::ConData
{
	struct ConnectionData
	{
		// connection data
		std::list<uint> lossList;
		uint lastLoss;
		uint averageLoss;
		std::list<uint> pingList;
		uint averagePing;
		uint pingFluctuation;
		uint lastPacketsSent;
		uint lastPacketsReceived;
		uint lastPacketsDropped;
		uint lags;
		std::list<uint> objUpdateIntervalsList;
		mstime lastObjUpdate;
		mstime lastObjTimestamp;

		// exception
		bool exception;
		std::string exceptionReason;

		// Client Id (for when receiving data)
		uint client;
	};

	struct ConnectionDataException final
	{
		ClientId client;
		bool isException;
		std::string reason;
	};

	// Inter plugin comms

	class ConDataCommunicator : public PluginCommunicator
	{
	  public:
		inline static const char* pluginName = "Advanced Connection Data";
		explicit ConDataCommunicator(const std::string& plug);

		void PluginCall(ReceiveException, const ConnectionDataException&);
		void PluginCall(ReceiveData, ConnectionData&);
	};

	//! The struct that holds client info for this plugin
	struct MiscClientInfo final
	{
		bool lightsOn = false;
		bool shieldsDown = false;
	};

	struct Config final : Reflectable
	{
		std::string File() override { return "config/condata.json"; }
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

		ConnectionData connections[MaxClientId + 1];

		ConDataCommunicator* communicator = nullptr;
		Tempban::TempBanCommunicator* tempBanCommunicator = nullptr;
	};
}; // namespace Plugins::ConData

REFL_AUTO(type(Plugins::ConData::Config), field(pingKick), field(pingKickFrame), field(fluctKick), field(lossKick), field(lossKickFrame), field(lagKick),
    field(lagDetectionFrame), field(lagDetectionMin), field(kickThreshold), field(allowPing))