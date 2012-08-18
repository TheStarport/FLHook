using System;
using System.Collections.Generic;
using System.IO;
using System.Runtime.InteropServices;
using System.ComponentModel;

namespace PlayerCntlSetup
{
    public class FLGameData
    {
        public GameDataSet DataStore { get; private set; }
        
        public const string GAMEDATA_AMMO = "ammo";
        public const string GAMEDATA_BASES = "bases";
        public const string GAMEDATA_CARGO = "cargo";
        public const string GAMEDATA_CM = "countermeasures";
        public const string GAMEDATA_ENGINES = "engines";
        public const string GAMEDATA_FACTIONS = "factions";
        public const string GAMEDATA_GUNS = "guns";
        public const string GAMEDATA_LIGHTS = "lights";
        public const string GAMEDATA_ARMOR = "armor";
        public const string GAMEDATA_MINES = "mines";
        public const string GAMEDATA_MISC = "miscequipment";
        public const string GAMEDATA_FX = "fx"; 
        public const string GAMEDATA_CLOAK = "cloak";
        public const string GAMEDATA_SOUND = "sound";
        public const string GAMEDATA_POWERGEN = "powergenerators";
        public const string GAMEDATA_PROJECTILES = "projectiles";
        public const string GAMEDATA_SCANNERS = "scanners";
        public const string GAMEDATA_SHIELDS = "shields";
        public const string GAMEDATA_SHIPS = "ships";
        public const string GAMEDATA_SYSTEMS = "systems";
        public const string GAMEDATA_THRUSTERS = "thrusters";
        public const string GAMEDATA_TRACTORS = "tractorbeams";
        public const string GAMEDATA_TURRETS = "turrets";
        public const string GAMEDATA_GEN = "genhash";
        public const string GAMEDATA_OBJECT = "object";
        public const string GAMEDATA_ZONE = "zone";
        public const string GAMEDATA_ROOM = "room";

        public FLGameData()
        {
            DataStore = new GameDataSet();
        }

        /// <summary>
        /// Unmanaged functions to access libraries
        /// </summary>
        static int DONT_RESOLVE_DLL_REFERENCES = 0x00000001;
        static int LOAD_LIBRARY_AS_DATAFILE = 0x00000002;
        [DllImport("kernel32.dll", SetLastError = true)]
        static extern IntPtr LoadLibraryExA(string lpLibFileName, int hFile, int dwFlags);

        [DllImport("user32.dll", SetLastError = true)]
        static extern int LoadString(IntPtr hInstance, int uID, byte[] lpBuffer, int nBufferMax);

        [DllImport("kernel32.dll", SetLastError = true)]
        static extern int FreeLibrary(IntPtr hInstance);

        [DllImport("kernel32.dll", SetLastError = true)]
        static extern int LockResource(int hResData);

        [DllImport("kernel32.dll")]
        static extern IntPtr FindResource(IntPtr hModule, int lpID, int lpType);

        [DllImport("kernel32.dll", SetLastError = true)]
        static extern IntPtr LoadResource(IntPtr hModule, IntPtr hResInfo);

        [DllImport("kernel32.dll", SetLastError = true)]
        static extern int SizeofResource(IntPtr hModule, IntPtr hResInfo);

        /// <summary>
        /// Resource dlls containing strings.
        /// </summary>
        List<IntPtr> vDLLs = new List<IntPtr>();

        private void LoadLibrary(string dllPath)
        {
            IntPtr hInstance = LoadLibraryExA(dllPath, 0, DONT_RESOLVE_DLL_REFERENCES | LOAD_LIBRARY_AS_DATAFILE);
            if (hInstance != null)
                vDLLs.Add(hInstance);
        }

        /// <summary>
        /// Return the string for the specified IDS. Note that this function
        /// works only for ascii strings.
        /// </summary>
        /// <param name="iIDS"></param>
        /// <returns>The string or null if it cannot be found.</returns>
        private string GetIDString(uint iIDS)
        {
            int iDLL = (int)(iIDS / 0x10000);
            int resId = (int)iIDS - (iDLL * 0x10000);

            if (vDLLs.Count > iDLL)
            {
                IntPtr hInstance = vDLLs[iDLL];
                if (hInstance != IntPtr.Zero)
                {
                    byte[] bufName = new byte[10000];
                    int len = LoadString(hInstance, resId, bufName, bufName.Length);
                    if (len > 0)
                    {
                        return System.Text.Encoding.ASCII.GetString(bufName, 0, len);
                    }

                    IntPtr hFindRes = FindResource(hInstance, resId, 23);
                    if (hFindRes != IntPtr.Zero)
                    {
                        IntPtr resContent = LoadResource(hInstance, hFindRes);
                        if (resContent != IntPtr.Zero)
                        {
                            int size = SizeofResource(hInstance, hFindRes);
                            byte[] bufInfo = new byte[size];
                            Marshal.Copy(resContent, bufInfo, 0, (int)size);
                            return System.Text.Encoding.Unicode.GetString(bufInfo, 0, size);
                        }
                    }
                }
            }

            return null;
        }

        private Dictionary<uint, uint> infocardMap = new Dictionary<uint, uint>();

        /// <summary>
        /// Load all game data.
        /// </summary>
        public void LoadAll(string flExePath, LogRecorderInterface log)
        {
            DataStore.Clear();

            FLDataFile flIni = null;
            try
            {
                flIni = new FLDataFile(flExePath + Path.DirectorySeparatorChar + "Freelancer.ini", true);
            }
            catch (Exception e)
            {
                log.AddLog("Error '" + e.Message + "' when parsing '" + flExePath);
                return;
            }
            string flDataPath = Path.GetFullPath(Path.Combine(flExePath, flIni.GetSetting("Freelancer", "data path").Str(0)));

            log.AddLog("Loading strings...");

            // Load the string dlls.
            LoadLibrary(flExePath + Path.DirectorySeparatorChar + "resources.dll");
            foreach (FLDataFile.Setting flIniEntry in flIni.GetSettings("Resources", "DLL"))
                LoadLibrary(flExePath + Path.DirectorySeparatorChar + flIniEntry.Str(0));

            log.AddLog("Loading universe...");

            // Scan ini files and parse base entries
            foreach (FLDataFile.Setting flIniEntry in flIni.GetSettings("Data", "universe"))
            {
                string iniPath = flDataPath + Path.DirectorySeparatorChar + flIniEntry.Str(0);
                try
                {
                    FLDataFile ini = new FLDataFile(iniPath, true);
                    foreach (FLDataFile.Section section in ini.sections)
                    {
                        try
                        {
                            string sectionName = section.sectionName.ToLowerInvariant();
                            if (sectionName == "base")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_BASES, true);
                                string baseNick = section.GetSetting("nickname").Str(0);
                                string baseFilePath = flDataPath + Path.DirectorySeparatorChar + section.GetSetting("file").Str(0);
                                LoadRoomData(DataStore, baseNick, baseFilePath);
                            }
                            else if (sectionName == "system")
                            {
                                string baseNick = section.GetSetting("nickname").Str(0).ToLowerInvariant();
                                AddGameData(DataStore.HashList, section, GAMEDATA_SYSTEMS, true);
                                string file = Directory.GetParent(ini.filePath).FullName + "\\" + section.GetSetting("file").Str(0);
                                ParseSystem(file, log);
                            }
                        }
                        catch (Exception e)
                        {
                            log.AddLog("Error '" + e.Message + "' when parsing '" + iniPath);
                        }
                    }
                }
                catch (Exception e)
                {
                    log.AddLog("Error '" + e.Message + "' when parsing '" + iniPath);
                }
            }

            log.AddLog("Loading ships...");

            // Scan ship files and parse ship entries
            foreach (FLDataFile.Setting flIniEntry in flIni.GetSettings("Data", "ships"))
            {
                string iniPath = flDataPath + Path.DirectorySeparatorChar + flIniEntry.Str(0);
                try
                {
                    FLDataFile ini = new FLDataFile(iniPath, true);
                    foreach (FLDataFile.Section section in ini.sections)
                    {
                        if (section.sectionName.ToLowerInvariant() == "ship")
                        {
                            AddGameData(DataStore.HashList, section, GAMEDATA_SHIPS, true);

                            string nickName = section.GetSetting("nickname").Str(0);
                            uint hash = FLUtility.CreateID(nickName.ToLowerInvariant());

                            foreach (FLDataFile.Setting setting in section.settings)
                            {
                                try
                                {
                                    if (setting.settingName.ToLowerInvariant() == "da_archetype")
                                    {
                                        // UTF hardpoints are only really useful to validate the shiparch and ship default loadout.
                                        // Don't bother loading them.
                                        // UtfFile utf = new UtfFile(flDataPath + Path.DirectorySeparatorChar + setting.Str(0));
                                        // foreach (string hp in utf.hardpoints)
                                        // {
                                        //    DataStore.HardPointList.AddHardPointListRow(hash, hp, "");
                                        // }
                                    }
                                    else if (setting.settingName.ToLowerInvariant() == "hp_type")
                                    {

                                        string type = setting.Str(0);
                                        for (int i = 1; i < setting.NumValues(); i++)
                                        {
                                            string hp = setting.Str(i);
                                            GameDataSet.HardPointListRow hpInfo = GetHardPointByShipAndHPName(hash, hp);
                                            if (hpInfo == null)
                                                hpInfo = DataStore.HardPointList.AddHardPointListRow(hash, hp, HardpointClassToGameDataClass(type), "", 0);
                                            hpInfo.MountableTypes += " " + type;
                                        }
                                    }
                                }
                                catch (Exception ex)
                                {
                                    log.AddLog(String.Format("Error '{0}' when reading {1}", ex.Message, setting.desc));
                                }
                            }
                        }
                    }
                }
                catch (Exception e)
                {
                    log.AddLog("Error '" + e.Message + "' when parsing '" + iniPath);
                }
            }

            log.AddLog("Loading equipment...");

            // Scan ini files and parse equipment entries.
            foreach (FLDataFile.Setting flIniEntry in flIni.GetSettings("Data", "equipment"))
            {
                string iniPath = flDataPath + Path.DirectorySeparatorChar + flIniEntry.Str(0);
                try
                {
                    FLDataFile ini = new FLDataFile(iniPath, true);
                    foreach (FLDataFile.Section section in ini.sections)
                    {
                        try
                        {
                            string sectionName = section.sectionName.ToLowerInvariant();
                            // Internal equipment; these are mounted internally.
                            if (sectionName == "engine")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_ENGINES, true);
                            }
                            else if (sectionName == "power")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_POWERGEN, true);
                            }
                            else if (sectionName == "scanner")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_SCANNERS, true);
                            }
                            else if (sectionName == "tractor")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_TRACTORS, true);
                            }
                            else if (sectionName == "cloakingdevice")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_CLOAK, true);
                            }
                            else if (sectionName == "armor")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_ARMOR, true);
                            }
                            else if (sectionName == "internalfx")
                            {
                                if (section.SettingExists("use_sound"))
                                {
                                    AddGameData(DataStore.HashList, section, GAMEDATA_SOUND, false);
                                }
                            }
                            // External hardpoints.
                            else if (sectionName == "attachedfx")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_FX, false);
                            }
                            else if (sectionName == "light")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_LIGHTS, false);
                            }
                            else if (sectionName == "gun")
                            {
                                if (section.SettingExists("hp_gun_type"))
                                {
                                    string hpType = section.GetSetting("hp_gun_type").Str(0);
                                    AddGameData(DataStore.HashList, section, HardpointClassToGameDataClass(hpType), true);
                                    DataStore.EquipInfoList.AddEquipInfoListRow(
                                        FLUtility.CreateID(section.GetSetting("nickname").Str(0)),
                                        HardpointClassToGameDataClass(hpType), hpType);
                                }
                                // Probably an npc gun
                                else
                                {
                                    AddGameData(DataStore.HashList, section, GAMEDATA_GEN, false);
                                }
                            }
                            else if (sectionName == "shieldgenerator")
                            {
                                if (section.SettingExists("hp_type"))
                                {
                                    string hpType = section.GetSetting("hp_type").Str(0);
                                    AddGameData(DataStore.HashList, section, HardpointClassToGameDataClass(hpType), true);
                                    DataStore.EquipInfoList.AddEquipInfoListRow(
                                        FLUtility.CreateID(section.GetSetting("nickname").Str(0)),
                                        HardpointClassToGameDataClass(hpType), hpType);
                                }
                                // Probably an npc shield
                                else
                                {
                                    AddGameData(DataStore.HashList, section, GAMEDATA_GEN, false);
                                }
                            }
                            else if (sectionName == "countermeasuredropper")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_CM, true);
                            }
                            else if (sectionName == "thruster")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_THRUSTERS, true);
                            }
                            else if (sectionName == "minedropper")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_MINES, true);
                            }

                            // Cargo and ammo.
                            else if (sectionName == "munition")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_AMMO, true);
                            }
                            else if (sectionName == "repairkit")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_CARGO, true);
                            }
                            else if (sectionName == "countermeasure")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_AMMO, true);
                            }
                            else if (sectionName == "shieldbattery")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_CARGO, true);
                            }
                            else if (sectionName == "mine")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_AMMO, true);
                            }
                            else if (sectionName == "commodity")
                            {
                                AddGameData(DataStore.HashList, section, GAMEDATA_CARGO, true);
                            }

                            // Random stuff I don't know what to do with.
                            else if (sectionName == "shield")
                            { } // ignore this section
                            else if (sectionName == "lootcrate")
                            { } // ignore this section
                            else if (sectionName == "lod")
                            { } // ignore this section - it has no nickname
                            else if (sectionName == "tradelane")
                            { } // ignore this section
                            else if (sectionName == "motor")
                            { } // ignore this section
                            else if (sectionName == "explosion")
                            { } // ignore this section
                            else if (sectionName == "cargopod")
                            { } // ignore this section
                            else if (sectionName == "tradelane")
                            { } // ignore this section

                            else
                            {
                            }
                        }
                        catch (Exception e)
                        {
                            log.AddLog("Error '" + e.Message + "' when parsing '" + iniPath);
                        }
                    }
                }
                catch (Exception e)
                {
                    log.AddLog("Error '" + e.Message + "' when parsing '" + iniPath);
                }
            }

            log.AddLog("Loading factions...");

            // Scan ini files and parse faction entries
            foreach (FLDataFile.Setting flIniEntry in flIni.GetSettings("Data", "groups"))
            {
                string iniPath = flDataPath + Path.DirectorySeparatorChar + flIniEntry.Str(0);
                try
                {
                    FLDataFile ini = new FLDataFile(iniPath, true);
                    foreach (FLDataFile.Section section in ini.sections)
                    {
                        if (section.sectionName.ToLowerInvariant() == "group")
                        {
                            AddGameData(DataStore.HashList, section, GAMEDATA_FACTIONS, true);
                        }
                    }
                }
                catch (Exception e)
                {
                    log.AddLog("Error '" + e.Message + "' when parsing '" + iniPath);
                }
            }
            foreach (IntPtr hInstance in vDLLs)
                FreeLibrary(hInstance);
            vDLLs.Clear();
        }

        private string GetIDSParm(FLDataFile.Section section, string parmName)
        {
            string stInfo = "";
            if (section.SettingExists(parmName))
            {
                uint idsInfo = section.GetSetting(parmName).UInt(0);
                stInfo = GetIDString(idsInfo);
                if (stInfo == null)
                    throw new Exception("ids_info not found " + idsInfo);
                stInfo = stInfo.Trim();
            }
            return stInfo;
        }

        private bool AddGameData(GameDataSet.HashListDataTable items, FLDataFile.Section section, string gameDataType, bool ignoreEntriesWithNoIds)
        {
            string nickName = section.GetSetting("nickname").Str(0);
            
            string stIDSName = GetIDSParm(section, "ids_name");
            if (stIDSName == "")
                stIDSName = GetIDSParm(section, "strid_name");

            if (ignoreEntriesWithNoIds && stIDSName == "")
                return false;

            uint hash = FLUtility.CreateID(nickName.ToLowerInvariant());
            if (gameDataType == GAMEDATA_FACTIONS)
                hash = FLUtility.CreateFactionID(nickName);

            GameDataSet.HashListRow conflictingHashEntry = items.FindByItemHash(hash);
            if (conflictingHashEntry != null)
                return false;

            string stIDSInfo = GetIDSParm(section, "ids_info");
            
            string stIDSInfo1 = GetIDSParm(section, "ids_info1");
            if (stIDSInfo1 == "")
                stIDSInfo1 = GetIDSParm(section, "ids_short_name");

            string stIDSInfo2 = GetIDSParm(section, "ids_info2");
            string stIDSInfo3 = GetIDSParm(section, "ids_info3");

            string keys = hash.ToString() + " 0x" + hash.ToString("X");

            items.AddHashListRow(hash, nickName, gameDataType, stIDSName, stIDSInfo, stIDSInfo1, stIDSInfo2, stIDSInfo3, keys);
            return true;
        }


        /// <summary>
        /// Parse the system file to extract base information.
        /// </summary>
        /// <param name="fileName"></param>
        void ParseSystem(string fileName, LogRecorderInterface log)
        {
            FLDataFile ini = new FLDataFile(fileName, true);
            foreach (FLDataFile.Section section in ini.sections)
            {
                string idsName = "";
                string idsInfo = "";
                string idsInfo1 = "";
                string pos = "";

                if (section.sectionName.ToLowerInvariant() == "object")
                {
                    try
                    {

                        if (section.SettingExists("ids_name"))
                        {
                            idsName = GetIDString(section.GetSetting("ids_name").UInt(0));
                        }

                        if (section.SettingExists("ids_info"))
                        {
                            uint value = section.GetSetting("ids_info").UInt(0);
                            idsInfo = GetIDString(value);
                            if (infocardMap.ContainsKey(value))
                                idsInfo1 = GetIDString(infocardMap[value]);
                        }

                        if (section.SettingExists("pos"))
                        {
                            pos = section.GetSetting("pos").Str(0)
                                + "," + section.GetSetting("pos").Str(1)
                                + "," + section.GetSetting("pos").Str(2);
                        }

                        if (section.SettingExists("base"))
                        {
                            string baseNick = section.GetSetting("base").Str(0);
                            GameDataSet.HashListRow baseItem = GetItemByNickName(baseNick);
                            if (baseItem != null)
                            {
                                if (baseItem.IDSInfo.Length == 0)
                                    baseItem.IDSInfo = idsInfo;
                                if (baseItem.IDSInfo1.Length == 0)
                                    baseItem.IDSInfo1 = idsInfo1;
                                if (baseItem.IDSInfo2.Length == 0)
                                    baseItem.IDSInfo2 = "Postion: " + pos;
                            }
                        }

                        AddGameData(DataStore.HashList, section, GAMEDATA_OBJECT, false);
                    }
                    catch (Exception e)
                    {
                        log.AddLog("Error '" + e.Message + "' when parsing '" + fileName);
                    }
                }
                else if (section.sectionName.ToLowerInvariant() == "zone")
                {
                    try
                    {
                        AddGameData(DataStore.HashList, section, GAMEDATA_ZONE, false);
                    }
                    catch (Exception e)
                    {
                        log.AddLog("Error '" + e.Message + "' when parsing '" + fileName);
                    }
                }
            }
        }


        /// <summary>
        /// Load room hash codes from base ini data.
        /// </summary>
        /// <param name="baseIniPath"></param>
        private void LoadRoomData(GameDataSet dataStore, string baseNick, string baseIniPath)
        {
            FLDataFile iniRoom = new FLDataFile(baseIniPath, true);
            foreach (FLDataFile.Section section in iniRoom.sections)
            {
                if (section.sectionName.ToLowerInvariant() != "room")
                    continue;

                string locationNick = FLUtility.CreateID(baseNick.ToLowerInvariant()).ToString("x")
                    + "_" + section.GetSetting("nickname").Str(0).ToLowerInvariant();

                uint hashU = FLUtility.CreateID(locationNick.ToLowerInvariant());
                string keys = hashU.ToString() + " 0x" + hashU.ToString("X");

                GameDataSet.HashListRow conflictingEntry = dataStore.HashList.FindByItemHash(hashU);
                if (conflictingEntry == null)
                {
                    dataStore.HashList.AddHashListRow(hashU, locationNick, GAMEDATA_ROOM, GetItemDescByNickNameX(baseNick), "", "", "", "", keys);
                }
            }   
        }

        /// <summary>
        /// Scan the goods files for the specifed shiphull to add the effects entries
        /// from the goods package to the ship as synthetic hardpoints.
        /// </summary>
        /// <param name="info"></param>
        /// <param name="loadout"></param>
        void AddLoadout(FLDataFile flIni, string flDataPath, FLDataFile.Section shipLoadoutSection, LogRecorderInterface log)
        {
            try
            {
                string shipNickName = shipLoadoutSection.GetSetting("archetype").Str(0);
                uint shiphash = FLUtility.CreateID(shipNickName);

                long defaultSound = 0;
                long defaultPowerPlant = 0;
                long defaultEngine = 0;

                foreach (FLDataFile.Setting setting in shipLoadoutSection.settings)
                {

                    try
                    {
                        if (setting.settingName.ToLowerInvariant() != "equip")
                            continue;

                        if (setting.NumValues() != 3)
                            throw new Exception(setting.desc + "setting '" + setting.desc + "' should have 3 values");

                        string equipNickName = setting.Str(0);
                        GameDataSet.HashListRow item = GetItemByNickName(equipNickName);
                        if (item == null)
                            throw new Exception(setting.desc + " cannot find " + equipNickName + " in game data table");

                        if (item.ItemType == GAMEDATA_LIGHTS)
                        {
                            if (setting.Str(1) == "")
                                throw new Exception(setting.desc + " invalid hardpoint for " + equipNickName);
                            DataStore.HardPointList.AddHardPointListRow(shiphash, setting.Str(1), item.ItemType, "", item.ItemHash);
                        }
                        else if (item.ItemType == GAMEDATA_FX)
                        {
                            if (setting.Str(1) == "")
                                throw new Exception(setting.desc + " invalid hardpoint for " + equipNickName);
                            DataStore.HardPointList.AddHardPointListRow(shiphash, setting.Str(1), item.ItemType, "", item.ItemHash);
                        }
                        else if (item.ItemType == GAMEDATA_SOUND)
                        {
                            defaultSound = item.ItemHash;
                        }
                        else if (item.ItemType == GAMEDATA_POWERGEN)
                        {
                            defaultPowerPlant = item.ItemHash;
                        }
                        else if (item.ItemType == GAMEDATA_ENGINES)
                        {
                            defaultEngine = item.ItemHash;
                        }
                    }
                    catch (Exception e)
                    {
                        log.AddLog("Error '" + e.Message + "'");
                    }
                }

                DataStore.ShipInfoList.AddShipInfoListRow(shiphash, defaultEngine, defaultSound, defaultPowerPlant);
            }
            catch (Exception e)
            {
                log.AddLog("Error '" + e.Message + "'");
            }
        }

        /// <summary>
        /// Scan the goods files for the specifed shiphull to add the effects entries
        /// from the goods package to the ship as synthetic hardpoints.
        /// </summary>
        /// <param name="info"></param>
        /// <param name="loadout"></param>
        void AddFXFromGoods(FLDataFile flGoodsIni, string flDataPath, FLDataFile.Section shipPackageSection, LogRecorderInterface log)
        {
            try
            {
                string hullNickName = shipPackageSection.GetSetting("hull").Str(0);

                foreach (FLDataFile.Section section in flGoodsIni.sections)
                {
                    if (section.sectionName.ToLowerInvariant() != "good")
                        continue;

                    if (section.GetSetting("category").Str(0)!="shiphull")
                        continue;

                    if (section.GetSetting("nickname").Str(0)!=hullNickName)
                        continue;

                    string shipNickName = section.GetSetting("ship").Str(0);
                    uint shiphash = FLUtility.CreateID(shipNickName);

                    long defaultSound = 0;
                    long defaultPowerPlant = 0;
                    long defaultEngine = 0;

                    // Get the loadout for the ship.
                    foreach (FLDataFile.Setting setting in shipPackageSection.settings)
                    {
                        try
                        {
                            if (setting.settingName.ToLowerInvariant() != "addon")
                                continue;

                            if (setting.NumValues() != 3)
                                throw new Exception(setting.desc + "setting '" + setting.desc + "' should have 3 values");

                            string equipNickName = setting.Str(0);
                            GameDataSet.HashListRow item = GetItemByNickName(equipNickName);
                            if (item == null)
                                throw new Exception(setting.desc + " cannot find " + equipNickName + " in game data table");

                            if (item.ItemType == GAMEDATA_LIGHTS)
                            {
                                if (setting.Str(1) == "internal")
                                    throw new Exception(setting.desc + " invalid hardpoint for " + equipNickName);
                                DataStore.HardPointList.AddHardPointListRow(shiphash, setting.Str(1), item.ItemType, "", item.ItemHash);
                            }
                            else if (item.ItemType == GAMEDATA_FX)
                            {
                                if (setting.Str(1) == "internal")
                                    throw new Exception(setting.desc + " invalid hardpoint for " + equipNickName);
                                DataStore.HardPointList.AddHardPointListRow(shiphash, setting.Str(1), item.ItemType, "", item.ItemHash);
                            }
                            else if (item.ItemType == GAMEDATA_SOUND)
                            {
                                defaultSound = item.ItemHash;
                            }
                            else if (item.ItemType == GAMEDATA_POWERGEN)
                            {
                                defaultPowerPlant = item.ItemHash;
                            }
                            else if (item.ItemType == GAMEDATA_ENGINES)
                            {
                                defaultEngine = item.ItemHash;
                            }
                        }
                        catch (Exception e)
                        {
                            log.AddLog("Error '" + e.Message + "'");
                        }
                    }

                    DataStore.ShipInfoList.AddShipInfoListRow(shiphash, defaultEngine, defaultSound, defaultPowerPlant);
                }
            }
            catch (Exception e)
            {
                log.AddLog("Error '" + e.Message + "'");
            }
        }

        /// <summary>
        /// Scans the specified directory and recursively search for ini files that appear to contain
        /// hashcodes.
        /// </summary>
        /// <param name="flDataPath"></param>
        /// <param name="iniFilePath"></param>
        /// <remarks>Not used any more</remarks>
        private void ScanForHashCodes(GameDataSet dataStore, string flDataPath, BackgroundWorker bgw, LogRecorderInterface log)
        {
            // Scan the ini files.    
            string[] iniFiles = Directory.GetFiles(flDataPath, "*.ini");
            for (int i=0; i<iniFiles.Length; i++)
            {
                if (bgw.CancellationPending)
                    return;

                string filePath = iniFiles[i];
                bgw.ReportProgress(((i * 100) / iniFiles.Length), "Scanning " + filePath.Substring(flDataPath.Length + 1) + "...");
                try
                {
                    FLDataFile ini = new FLDataFile(filePath, true);
                    foreach (FLDataFile.Section section in ini.sections)
                    {
                        foreach (FLDataFile.Setting e in section.settings)
                        {
                            if (e.NumValues() == 0)
                                continue;

                            string st = e.settingName.ToLowerInvariant();
                            if (st == "nickname" || st == "archetype" || st == "loadout"
                                || st == "explosion_arch" || st == "fuse" || st == "zone"
                                || st == "name" || st == "room" || st == "prop"
                                || st == "msg_id_prefix" || st == "npc")
                            {
                                string nickName = e.Str(0);
                                uint hashU = FLUtility.CreateID(nickName.ToLowerInvariant());
                                GameDataSet.HashListRow conflictingEntry = dataStore.HashList.FindByItemHash(hashU);
                                if (conflictingEntry == null)
                                {
                                    string keys = hashU.ToString() + " 0x" + hashU.ToString("X");
                                    dataStore.HashList.AddHashListRow(hashU, nickName, GAMEDATA_GEN, "", "", "", "", "", keys);
                                }
                            }

                            /* for (int j = 0; j < e.NumValues(); j++)
                            {
                                string nickName = e.Str(j);
                                uint hashU = FLUtility.CreateID(nickName.ToLowerInvariant());
                                string keys = hashU.ToString() + " 0x" + hashU.ToString("X");
                                GameDataSet.HashListRow conflictingEntry = dataStore.HashList.FindByItemHash(hashU);
                                if (conflictingEntry == null)
                                {
                                    dataStore.HashList.AddHashListRow(hashU, nickName, GAMEDATA_GEN, "", "", "", "", "");
                                }
                            } */                       
                        }
                    }
                }
                catch (Exception ex)
                {
                    log.AddLog(String.Format("Error '{0}' when scanning {1}", ex.Message, filePath));
                }
            }
            
            // Recurse through subdirectories.
            string[] dataDirs = Directory.GetDirectories(flDataPath);
            for (int i = 0; i < dataDirs.Length; i++ )
            {
                ScanForHashCodes(dataStore, dataDirs[i], bgw, log);
            }
        }


        /// <summary>
        /// Return the item for the specified faction nickname.
        /// </summary>
        /// <param name="itemHash">The nickname.</param>
        /// <returns>The item or null if it cannot be found.</returns>
        public GameDataSet.HashListRow GetItemByFactionNickName(string nick)
        {
            return GetItemByHash(FLUtility.CreateFactionID(nick.ToLowerInvariant()));
        }

        /// <summary>
        /// Return the item for the specified nickname.
        /// </summary>
        /// <param name="itemHash">The nickname.</param>
        /// <returns>The item or null if it cannot be found.</returns>
        public GameDataSet.HashListRow GetItemByNickName(string nick)
        {
            return GetItemByHash(FLUtility.CreateID(nick.ToLowerInvariant()));
        }

        /// <summary>
        /// Return a human readable description for a nickname.
        /// </summary>
        /// <param name="itemHash">The nickname</param>
        /// <returns>A description string.</returns>
        public string GetItemDescByFactionNickName(string nick)
        {
            GameDataSet.HashListRow item = GetItemByFactionNickName(nick);
            if (item == null)
                return "Unknown[" + nick + "]";
            return item.IDSName;
        }

        /// <summary>
        /// Return a human readable description for a nickname.
        /// </summary>
        /// <param name="itemHash">The nickname</param>
        /// <returns>A description string.</returns>
        public string GetItemDescByNickNameX(string nick)
        {
            GameDataSet.HashListRow item = GetItemByNickName(nick);
            if (item==null)
                return "Unknown[" + nick + "]";
            return item.IDSName;
        }


        /// <summary>
        /// Return the item for the specified key or hashcode.
        /// </summary>
        /// <param name="itemHash">The hashcode or key.</param>
        /// <returns>The item or null if it cannot be found.</returns>
        public GameDataSet.HashListRow GetItemByHash(long itemHash)
        {
            return DataStore.HashList.FindByItemHash(itemHash);
        }

        /// <summary>
        /// Return a human readable description for a key from a character file.
        /// </summary>
        /// <param name="itemHash">The hashcode or key.</param>
        /// <returns>A description string.</returns>
        public string GetItemDescByHash(long itemHash)
        {
            GameDataSet.HashListRow item = GetItemByHash(itemHash);
            if (item == null)
                return "Unknown[" + itemHash + "]";
            if (item.IDSName.Length == 0)
                return item.ItemNickName;
            return item.IDSName;
        }

        /// <summary>
        /// Return the nickName for the hashcode from a character file.
        /// </summary>
        /// <param name="itemHash">The key or hashcode</param>
        /// <returns>The nickname string.</returns>
        public string GetItemNickByHash(long itemHash)
        {
            GameDataSet.HashListRow item = GetItemByHash(itemHash);
            if (item == null)
                return "Unknown[" + itemHash + "]";
            return item.ItemNickName;
        }

        /// <summary>
        /// Load an ion cross game data file that has a nickname key
        /// </summary>
        /// <param name="items">The database to copy data into</param>
        /// <param name="ioncrossDir">The file </param>
        /// <param name="hashFile"></param>
        private void LoadIonCrossNickNameDesc(GameDataSet.HashListDataTable items, string filePath, LogRecorderInterface log, bool isFaction)
        {
            try
            {
                FLDataFile ini = new FLDataFile(filePath, false);
                foreach (FLDataFile.Section s in ini.sections)
                {
                    foreach (FLDataFile.Setting setting in s.settings)
                    {
                        try
                        {
                            GameDataSet.HashListRow itemRecord = GetItemByNickName(setting.settingName.ToLowerInvariant());
                            if (isFaction)
                                itemRecord = GetItemByFactionNickName(setting.settingName.ToLowerInvariant());

                            if (itemRecord != null)
                            {
                                if (setting.NumValues() > 0)
                                    itemRecord.IDSName = setting.Str(0);
                            }
                        }
                        catch (Exception ex)
                        {
                            log.AddLog(String.Format("Error '{0}' when reading {1}", ex.Message, setting.desc));
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                log.AddLog(String.Format("Error '{0}' when reading {1}", ex.Message, filePath));
            }
        }

        /// <summary>
        /// Load a ion cross game data file that has a hash key
        /// </summary>
        /// <param name="items">The database to copy data into</param>
        /// <param name="ioncrossDir">The file </param>
        /// <param name="hashFile"></param>
        private void LoadIonCrossHashDesc(GameDataSet.HashListDataTable items, string filePath, LogRecorderInterface log)
        {
            try
            {
                FLDataFile ini = new FLDataFile(filePath, true);
                foreach (FLDataFile.Section s in ini.sections)
                {
                    foreach (FLDataFile.Setting setting in s.settings)
                    {
                        try
                        {
                            GameDataSet.HashListRow itemRecord = GetItemByNickName(setting.settingName);
                            if (itemRecord == null)
                                itemRecord = GetItemByHash(Convert.ToUInt32(setting.settingName));
                            if (itemRecord != null)
                            {
                                if (setting.NumValues() > 1)
                                    itemRecord.IDSName = setting.Str(1);
                                else if (setting.NumValues() > 0)
                                    itemRecord.IDSName = setting.Str(0);
                            }
                        }
                        catch (Exception ex)
                        {
                            log.AddLog(String.Format("Error '{0}' when reading {1}", ex.Message, setting.desc));
                        }
                    }
                }
            }
            catch (Exception ex)
            {
                log.AddLog(String.Format("Error '{0}' when reading {1}", ex.Message, filePath));
            }
        }

        private string HardpointClassToGameDataClass(string hpClass)
        {
            if (hpClass == "hp_cargo_pod")
                return GAMEDATA_MISC;

            if (hpClass == "hp_countermeasure_dropper")
                return GAMEDATA_CM;

            if (hpClass == "hp_turret")
                return GAMEDATA_TURRETS;

            if (hpClass == "hp_mine_dropper")
                return GAMEDATA_MINES;

            if (hpClass == "hp_torpedo")
                return GAMEDATA_PROJECTILES;

            if (hpClass == "hp_thruster")
                return GAMEDATA_THRUSTERS;

            if (hpClass == "hp_freighter_shield_generator")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_elite_shield_generator")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_fighter_shield_generator")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_shield_generator")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_gun")
                return GAMEDATA_GUNS;

            if (hpClass == "hp_child")
                return GAMEDATA_MISC;

            if (hpClass == "hp_tractor_source")
                return GAMEDATA_MISC;

            if (hpClass == "hp_bay_external")
                return GAMEDATA_MISC;

            if (hpClass == "hp_bay_surface")
                return GAMEDATA_MISC;

            if (hpClass == "hp_gun_special_1")
                return GAMEDATA_GUNS;

            if (hpClass == "hp_gun_special_2")
                return GAMEDATA_GUNS;

            if (hpClass == "hp_gun_special_3")
                return GAMEDATA_GUNS;

            if (hpClass == "hp_gun_special_4")
                return GAMEDATA_GUNS;

            if (hpClass == "hp_gun_special_5")
                return GAMEDATA_GUNS;

            if (hpClass == "hp_gun_special_6")
                return GAMEDATA_GUNS;

            if (hpClass == "hp_gun_special_7")
                return GAMEDATA_GUNS;

            if (hpClass == "hp_gun_special_8")
                return GAMEDATA_GUNS;

            if (hpClass == "hp_gun_special_9")
                return GAMEDATA_GUNS;

            if (hpClass == "hp_gun_special_10")
                return GAMEDATA_GUNS;

            if (hpClass == "hp_turret_special_1")
                return GAMEDATA_TURRETS;

            if (hpClass == "hp_turret_special_2")
                return GAMEDATA_TURRETS;

            if (hpClass == "hp_turret_special_3")
                return GAMEDATA_TURRETS;

            if (hpClass == "hp_turret_special_4")
                return GAMEDATA_TURRETS;

            if (hpClass == "hp_turret_special_5")
                return GAMEDATA_TURRETS;

            if (hpClass == "hp_turret_special_6")
                return GAMEDATA_TURRETS;

            if (hpClass == "hp_turret_special_7")
                return GAMEDATA_TURRETS;

            if (hpClass == "hp_turret_special_8")
                return GAMEDATA_TURRETS;

            if (hpClass == "hp_turret_special_9")
                return GAMEDATA_TURRETS;

            if (hpClass == "hp_turret_special_10")
                return GAMEDATA_TURRETS;

            if (hpClass == "hp_torpedo_special_1")
                return GAMEDATA_PROJECTILES;

            if (hpClass == "hp_torpedo_special_2")
                return GAMEDATA_PROJECTILES;

            if (hpClass == "hp_fighter_shield_special_1")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_fighter_shield_special_2")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_fighter_shield_special_3")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_fighter_shield_special_4")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_fighter_shield_special_5")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_fighter_shield_special_6")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_fighter_shield_special_7")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_fighter_shield_special_8")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_fighter_shield_special_9")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_fighter_shield_special_10")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_elite_shield_special_1")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_elite_shield_special_2")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_elite_shield_special_3")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_elite_shield_special_4")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_elite_shield_special_5")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_elite_shield_special_6")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_elite_shield_special_7")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_elite_shield_special_8")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_elite_shield_special_9")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_elite_shield_special_10")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_freighter_shield_special_1")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_freighter_shield_special_2")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_freighter_shield_special_3")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_freighter_shield_special_4")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_freighter_shield_special_5")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_freighter_shield_special_6")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_freighter_shield_special_7")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_freighter_shield_special_8")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_freighter_shield_special_9")
                return GAMEDATA_SHIELDS;

            if (hpClass == "hp_freighter_shield_special_10")
                return GAMEDATA_SHIELDS;

            throw new Exception("unknown hardpoint class " + hpClass);
        }

        public GameDataSet.ShipInfoListRow GetShipInfo(uint hash)
        {
            return DataStore.ShipInfoList.FindByShipHash(hash);
        }

        public GameDataSet.EquipInfoListRow GetEquipInfo(uint hash)
        {
            return DataStore.EquipInfoList.FindByEquipHash(hash);
        }

        public GameDataSet.HardPointListRow[] GetHardPointListByShip(uint shipHash)
        {
            string query = String.Format("ShipHash = '{0}'", shipHash);
            return (GameDataSet.HardPointListRow[]) DataStore.HardPointList.Select(query);
        }

        public GameDataSet.HardPointListRow GetHardPointByShipAndHPName(uint shipHash, string hpName)
        {
        
            string query = String.Format("ShipHash = '{0}' AND HPName = '{1}'", shipHash, hpName);
            GameDataSet.HardPointListRow[] hps = (GameDataSet.HardPointListRow[]) DataStore.HardPointList.Select(query);
            if (hps.Length > 1)
                throw new Exception("Invalid hardpoint configuration ship=" + shipHash + " hp=" + hpName);
            if (hps.Length == 0)
                return null;
            return hps[0];

        }
    }
}
