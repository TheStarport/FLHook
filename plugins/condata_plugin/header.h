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
