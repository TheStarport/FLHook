#pragma once

#include <FLHook.h>
#include <plugin.h>
#include <plugin_comms.h>
#include "../hookext_plugin/hookext_exports.h"

ReturnCode returncode;

/** the data for a single online player */
class INFO {
  public:
    INFO()
        : ulastPmClientID(-1), uTargetClientID(-1), bShowChatTime(false),
          bGreetingShown(false), iSwearWordWarnings(0) {}

    static const int NUMBER_OF_SLOTS = 10;
    std::wstring slot[NUMBER_OF_SLOTS];

    // Client ID of last PM.
    uint ulastPmClientID;

    // Client ID of selected target
    uint uTargetClientID;

    // Current chat time settings
    bool bShowChatTime;

    // True if the login banner has been displayed.
    bool bGreetingShown;

    // Swear word warn level
    int iSwearWordWarnings;
};

/** cache of preset messages for the online players (by client ID) */
static std::map<uint, INFO> mapInfo;

/** help text for when user types /help */
static std::list<INISECTIONVALUE> set_lstHelpLines;

/** greetings text for when user types /help */
static std::list<std::wstring> set_lstGreetingBannerLines;

/** special banner text for when user types /help */
static std::list<std::wstring> set_lstSpecialBannerLines;

/** special banner text for when user types /help */
static std::vector<std::list<std::wstring>> set_vctStandardBannerLines;

/** Time in second to repeat display of special banner */
static int set_iSpecialBannerTimeout;

/** Time in second to repeat display of standard banner */
static int set_iStandardBannerTimeout;

/** true if we override flhook built in help */
static bool set_bCustomHelp;

/** true if we echo user and admin commands to sender */
static bool set_bCmdEcho;

/** true if we don't echo mistyped user and admin commands to other players. */
static bool set_bCmdHide;

/** if true support the /showmsg and /setmsg commands */
static bool set_bSetMsg;

// Enable /me and /do commands
bool set_bEnableMe = false;
bool set_bEnableDo = false;

/** This parameter is sent when we send a chat time line so that we don't print
a time chat line recursively. */
static bool bSendingTime = false;

/** color of echoed commands */
static std::wstring set_wscCmdEchoStyle;

static std::wstring set_wscDisconnectSwearingInSpaceMsg;

static float set_fDisconnectSwearingInSpaceRange;

/** list of swear words */
static std::list<std::wstring> set_lstSwearWords;

/** A random macro to make things easier */
#define HAS_FLAG(a, b) ((a).wscFlags.find(b) != -1)

void UserCmd_ReplyToLastPMSender(uint iClientID, const std::wstring &wscParam);
void UserCmd_SendToLastTarget(uint iClientID, const std::wstring &wscParam);