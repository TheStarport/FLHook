#pragma once

#include <FLHook.h>
#include <plugin.h>
#include <FLCoreServer.h>

IMPORT unsigned int MakeLocationID(unsigned int, char const *);

ReturnCode returncode;

// Intro messages when entering the room.
static std::wstring set_wscIntroMsg1 =
    L"Pimp-my-ship facilities are available here.";
static std::wstring set_wscIntroMsg2 =
    L"Type /pimpship on your console to see options.";

// Cost per changed item.
static int set_iCost = 0;

// List of dealer rooms
static std::map<uint, std::wstring> set_mapDealers;

// Item of equipment for a single client.
struct EQ_HARDPOINT {
    EQ_HARDPOINT() : sID(0), iArchID(0), iOrigArchID(0) {}

    uint sID;
    uint iArchID;
    uint iOrigArchID;
    std::wstring wscHardPoint;
};

// List of connected clients.
struct INFO {
    INFO() : bInPimpDealer(false) {}

    // Map of hard point ID to equip.
    std::map<uint, EQ_HARDPOINT> mapCurrEquip;

    bool bInPimpDealer;
};
static std::map<uint, INFO> mapInfo;

// Map of item id to ITEM INFO
struct ITEM_INFO {
    ITEM_INFO() : iArchID(0) {}

    uint iArchID;
    std::wstring wscNickname;
    std::wstring wscDescription;
};
std::map<uint, ITEM_INFO> mapAvailableItems;
