#include "FLHook.h"
#include "plugin.h"
#include "../tempban_plugin/Main.h"

#define LOSS_INTERVALL 4000

struct CONNECTION_DATA {
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

struct CONNECTION_DATA_EXCEPTION {
    uint iClientID;
    bool bException;
    std::string sReason;
};

uint set_iPingKickFrame;
uint set_iPingKick;
uint set_iFluctKick;
uint set_iLossKickFrame;
uint set_iLossKick;
uint set_iLagDetectionFrame;
uint set_iLagDetectionMinimum;
uint set_iLagKick;

// Kick high lag and loss players only if the server load
// exceeds this threshold.
uint set_iKickThreshold;

// Inter plugin comms

class ConDataCommunicator : public PluginCommunicator {
  public:
    inline static const char *pluginName = "Advanced Connection Data Plugin by w0dk4";
    explicit ConDataCommunicator(std::string plug);

    HK_ERROR PluginCall(ReceiveException, CONNECTION_DATA_EXCEPTION);
    HK_ERROR PluginCall(ReceiveData, CONNECTION_DATA);
};
