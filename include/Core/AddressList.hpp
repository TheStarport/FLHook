#pragma once
enum class AddressList : uint
{
    Absolute = 0,

    // FLServer.exe Offsets

    Update = 0x1BAB4,
    Startup = 0x1BABC,
    Shutdown = 0x1BAB8,
    CharFileEncryption = 0x6E10D,
    CharFileEncryption2 = 0x6BFA6,
    CrcAntiCheat = 0x6FAF0,
    CreateChar = 0x6B790,
    FlNew = 0x80012,
    ServerFlServer = 0x1BC90,
    DisableNpcSpawns1 = 0x5987B,
    DisableNpcSpawns2 = 0x59CD3,
    DataPtr = 0x277EC,

    // Remove Client Offsets

    RcDisconnect = 0x93E0,
    RcSendChat = 0x7F30,
    CpList = 0x43D74,

    // Dalib.dll Offsets

    DalibDiscSuppress = 0x49C6,
    CdpServer = 0xA284,

    // Server.dll Offsets

    GetCommodities = 0x32EC2,
    MaxGroupSize = 0x3A068,
    MaxGroupSize2 = 0x3A46E,
    GetInspect = 0x206C0,
    GetFlName = 0x66370,
    PlayerDbMaxPlayersPatch = 0x64BC3,
    PlayerDbMaxPlayers = 0xB0264,
    RepArrayFree = 0x7F3F0,
    SaveFileHouseEntrySaveAndLoad = 0x679C6,
    SaveFileHouseEntrySaveAndLoadPatch = 0x78B39,
    SolarRefireRateBug = 0x02C057,
    FixNpcAnnouncer = 0x173da,
    UpdateCharacterFile = 0x6c547,
    UpdateCharacterFile2 = 0x6c9cd,
    SaveCharacter = 0x7EFA8,
    BannedFileCheck = 0x76b3e,
    ReadCharacterName1 = 0x72fe0,
    ReadCharacterName2 = 0x717be,
    CreateSolar = 0x2A62A,
    RemoveDefaultDeathMessages = 0x39124,
    ShipDestroyedInvoke = 0xB512,
    CObjectFind = 0x84464,
    GetObjectInspect1 = 0x87CD4,
    GetObjectInspect2 = 0x2074A,
    GetObjectInspect3 = 0x207BF,

    // Common.dll Offsets

    CommonVfTablePower = 0x1398F4,
    CommonVfTableScanner = 0x139920,
    CommonVfTableLight = 0x13994C,
    CommonVfTableTractor = 0x139978,
    CommonVfTableMine = 0x139C64,
    CommonVfTableCm = 0x139C90,
    CommonVfTableGun = 0x139C38,
    CommonVfTableShieldGen = 0x139BB4,
    CommonVfTableThruster = 0x139BE0,
    CommonVfTableShieldBat = 0x1399FC,
    CommonVfTableNanobot = 0x1399D0,
    CommonVfTableMunition = 0x139CE8,
    CommonVfTableEngine = 0x139AAC,
    CommonCObjDestructor = 0x4F45D,
CommonCObjectAllocator = 0x4EE50,
};
