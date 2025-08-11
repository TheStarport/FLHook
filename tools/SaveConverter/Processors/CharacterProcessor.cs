using System.Numerics;
using System.Text;
using FLHook.SaveConverter.Database;
using LibreLancer;
using LibreLancer.Ini;
using Serilog;

namespace FLHook.SaveConverter.Processors;

public static class CharacterProcessor
{
    public static Character? ProcessCharacter(string path, string accId)
    {
        Log.Information("Processing character: {dir}", Path.GetFileName(path));
        using var stream = File.OpenRead(path);
        
        IIniParser iniParser = new BinaryIniParser();
        if (!iniParser.CanParse(stream))
        {
            stream.Seek(0,  SeekOrigin.Begin);
            iniParser = new TextIniParser();
        }
        
        var sections = iniParser.ParseIniFile(path, stream, false);
        if (sections is null)
        {
            Log.Error("Couldn't read character file: {path}", path);
            Interlocked.Increment(ref Program.CharactersSkipped);
            return null;
        }

        var character = new Character()
        {
            AccountId = accId,
            CharacterName = ""
        };

        try
        {
            foreach (var section in sections)
            {
                if (section.Name.Equals("player", StringComparison.InvariantCultureIgnoreCase))
                {
                    if (!ProcessPlayerSection(section, character))
                    {
                        Interlocked.Increment(ref Program.CharactersSkipped);
                        return null;
                    }
                }
                else if (section.Name.Equals("mplayer", StringComparison.InvariantCultureIgnoreCase))
                {
                    if (!ProcessMPlayerSection(section, character))
                    {
                        Interlocked.Increment(ref Program.CharactersSkipped);
                        return null;
                    }
                }
            }
        }
        catch (Exception ex)
        {
            Log.Error(ex, "Error while reading character file: {path}", path);
            Interlocked.Increment(ref Program.CharactersSkipped);
            return null;
        }
        
        if (!string.IsNullOrEmpty(character.CharacterName))
        {
            Interlocked.Increment(ref Program.CharactersProcessed);
            return character;
        }
        
        Log.Error("Character file did not have character information, likely corrupted: {path}", path);
        Interlocked.Increment(ref Program.CharactersSkipped);
        return null;

    }

    private static bool ProcessPlayerSection(Section section, Character character)
    {
        var config = Program.Config;
        foreach (var entry in section)
        {
            switch (entry.Name.ToLowerInvariant())
            {
                case "name":
                    if (entry.Count < 1)
                    {
                        Log.Information("(Skipping) character has no name.");
                        return false;
                    }
                    
                    character.CharacterName = Encoding.BigEndianUnicode.GetString(Convert.FromHexString(entry[0].ToString()!));
                    break;
                case "rank":
                    character.Rank = entry.Count > 0 ? entry[0].ToInt32() : 0;
                    break;
                case "system":
                    character.System = entry.Count > 0 ? Program.Hash(entry[0].ToString()!) : 0;
                    break;
                case "rep_group":
                    character.RepGroup = entry.Count > 0 ? entry[0].ToString() : null;
                    break;
                case "house":
                    if (entry.Count is not 2)
                    {
                        Log.Warning("(Skipping entry) Character had reputation that didn't have exactly 2 parts.");
                        break;
                    }
                    
                    var rep = entry[0].ToSingle();
                    var faction = entry[1].ToString()!;

                    character.Reputation[faction] = rep;
                    break;
                case "money":
                    character.Money = Math.Max(entry.Count > 0 ? entry[0].ToInt32() : 0, 0);
                    break;
                case "num_kills":
                    character.KillCount = entry.Count > 0 ? entry[0].ToInt32() : 0;
                    break;
                case "num_misn_successes":
                    character.MissionSuccessCount = entry.Count > 0 ? entry[0].ToInt32() : 0;
                    break;
                case "num_misn_failures":
                    character.MissionFailureCount = entry.Count > 0 ? entry[0].ToInt32() : 0;
                    break;
                case "voice":
                    character.Voice = entry.Count > 0 ? entry[0].ToString()! : "trent_voice";
                    break;
                case "com_head":
                    character.CommCostume ??= new();
                    character.CommCostume.Head = entry.Count > 0 ? entry[0].ToInt32() : 0;
                    break;
                case "com_body":
                    character.CommCostume ??= new();
                    character.CommCostume.Body = entry.Count > 0 ? entry[0].ToInt32() : 0;
                    break;
                case "com_lefthand":
                    character.CommCostume ??= new();
                    character.CommCostume.LeftHand = entry.Count > 0 ? entry[0].ToInt32() : 0;
                    break;
                case "com_righthand":
                    character.CommCostume ??= new();
                    character.CommCostume.RightHand = entry.Count > 0 ? entry[0].ToInt32() : 0;
                    break;
                case "head":
                    character.BaseCostume ??= new();
                    character.BaseCostume.Head = entry.Count > 0 ? entry[0].ToInt32() : 0;
                    break;
                case "body":
                    character.BaseCostume ??= new();
                    character.BaseCostume.Body = entry.Count > 0 ? entry[0].ToInt32() : 0;
                    break;
                case "lefthand":
                    character.BaseCostume ??= new();
                    character.BaseCostume.LeftHand = entry.Count > 0 ? entry[0].ToInt32() : 0;
                    break;
                case "righthand":
                    character.BaseCostume ??= new();
                    character.BaseCostume.RightHand = entry.Count > 0 ? entry[0].ToInt32() : 0;
                    break;
                case "pos":
                    if (entry.Count >= 3)
                    {
                        character.Pos = [entry[0].ToSingle(), entry[1].ToSingle(), entry[2].ToSingle()];
                    }
                    break;
                case "ship_archetype":
                    character.ShipHash = entry.Count > 0 ? entry[0].ToInt32() : 0;
                    break;
                case "rotate":
                    if (entry.Count >= 3)
                    {
                        character.Rot = [entry[0].ToSingle(), entry[1].ToSingle(), entry[2].ToSingle()];
                    }
                    break;
                case "base":
                    character.CurrentBase = entry.Count > 0 ? Program.Hash(entry[0].ToString()!) : 0;
                    break;
                case "last_base":
                    character.LastDockedBase = entry.Count > 0 ? Program.Hash(entry[0].ToString()!) : 0;
                    break;
                case "hull_status":
                    character.HullStatus = entry.Count > 0 ? entry[0].ToSingle() : 1f;
                    break;
                case "base_hull_status":
                    character.BaseHullStatus = entry.Count > 0 ? entry[0].ToSingle() : 1f;
                    break;
                case "base_equip":
                case "equip":
                {
                    if (entry.Count != 3)
                    {
                        Log.Warning("(Skipping Entry) Ship had equipment that didn't have exactly 3 parts");
                        break;
                    }

                    var item = entry[0].ToInt32();
                    var hp = entry[1].ToString();
                    var health = entry[2].ToSingle();

                    var equipment = new Equipment()
                    {
                        ArchId = item,
                        HardPoint = string.IsNullOrWhiteSpace(hp) ? "BAY" : hp,
                        Health = health
                    };

                    var list = entry.Name.ToLowerInvariant().Equals("equip", StringComparison.InvariantCultureIgnoreCase) ? character.Equipment
                        : character.BaseEquipment;
                    list.Add(equipment);
                    break;
                }
                case "cargo":
                case "base_cargo":
                {
                    if (entry.Count != 5)
                    {
                        Log.Warning("(Skipping Entry) Ship had cargo that didn't have exactly 5 parts");
                        break;
                    }

                    var item = entry[0].ToInt32();
                    var amount = entry[1].ToInt32();
                    if (!float.TryParse(entry[3].ToString(), out var health))
                    {
                        health = 1f;
                    }
                    
                    var isMission = entry[4].ToInt32() is not 0;

                    var cargo = new Cargo()
                    {
                        ArchId = item,
                        Amount = amount,
                        Health = health,
                        IsMissionCargo = isMission,
                    };

                    var list = entry.Name.ToLowerInvariant().Equals("cargo", StringComparison.InvariantCultureIgnoreCase) ? character.Cargo
                        : character.BaseCargo;
                    list.Add(cargo);
                    break;
                }
                case "wg":
                {
                    if (entry.Count >= 2)
                    {
                        var group = entry[0].ToString()!;
                        var hp = entry[1].ToString()!;

                        if (character.WeaponGroups.TryGetValue(group, out var value))
                        {
                            value.Add(hp);
                        }
                        else
                        {
                            character.WeaponGroups.Add(group, [hp]);
                        }
                    }
                }
                    break;
                case "visit":
                    if (entry.Count >= 2)
                    {
                        character.Visits.Add([entry[0].ToInt32(), entry[1].ToInt32()]);
                    }
                    break;
                case "interface":
                    character.InterfaceState = entry.Count > 0 ? entry[0].ToInt32() : 0;
                    break;
            }
        }

        if (config.SkipZeroRankCharacters && character.Rank is 0)
        {
            Log.Warning("(Skipping) character has rank of zero.");
            return false;
        }

        if (config.CleanBadData)
        {
            if (character.RepGroup is not null &&
                Program.GameData.FactionProps.FactionProps.FirstOrDefault(x => x.Affiliation.Equals(character.RepGroup, StringComparison.InvariantCultureIgnoreCase)) is null)
            {
                Log.Information("(Fixing) Character had invalid rep_group, clearing");
                character.RepGroup = null;
            }

            if (character.System is 0 ||
                Program.GameData.Universe.Systems.FirstOrDefault(x => Program.Hash(x.Nickname) == character.System) is null)
            {
                Log.Information("(Fixing) Character had invalid system, setting to {system}", config.BrokenCharacterSystem);
                character.System = Program.Hash(config.BrokenCharacterSystem);
            }

            if (character.LastDockedBase is 0 ||
                Program.GameData.Universe.Bases.FirstOrDefault(x => Program.Hash(x.Nickname) == character.LastDockedBase) is null)
            {
                Log.Information("(Fixing) Character had invalid last base, setting to {base}", config.BrokenCharacterBase);
                character.LastDockedBase = Program.Hash(config.BrokenCharacterBase);
            }
        }

        if (config.ValidateCharacters)
        {
            if (character.System is 0)
            {
                Log.Warning("(Skipping) Character had no system");
                return false;
            }

            if (character.CharacterName.Length is 0)
            {
                Log.Warning("(Skipping) Character had no name");
                return false;
            }
            
            if (character.ShipHash is 0)
            {
                Log.Warning("(Skipping) Ship entry was not valid");
                return false;
            }

            if (Program.GameData.Ships.Ships.FirstOrDefault(x => Program.Hash(x.Nickname) == character.ShipHash) is null)
            {
                Log.Warning("(Skipping) Ship entry was found but did not match any ships");
                return false;
            }
        }

        return true;
    }
    
    private static bool ProcessMPlayerSection(Section section, Character character)
    {
        var config = Program.Config;
        foreach (var entry in section)
        {
            switch (entry.Name.ToLowerInvariant())
            {
                case "ship_type_killed":
                {
                    if (entry.Count is not 2)
                    {
                        break;
                    }
                    
                    var type = entry[0].ToString()!;
                    var newValue = entry[1].ToInt32()!;

                    if (character.ShipTypesKilled.TryGetValue(type, out var value))
                    {
                        character.ShipTypesKilled[type] = value + newValue;
                    }
                    else
                    {
                        character.ShipTypesKilled.Add(type, newValue);
                    }
                    break;
                }
                case "total_time_played":
                    character.TotalTimePlayed = Math.Max(entry.Count > 0 ? entry[0].ToInt32() : 0, 0);
                    break;
                case "rm_aborted":
                {
                    if (entry.Count is not 2)
                    {
                        break;
                    }
                    
                    var type = entry[0].ToString()!;
                    var amount = entry[1].ToInt32()!;
                    character.RandomMissionsAborted[type] = amount;
                    break;
                }
                case "rm_failed":
                {
                    if (entry.Count is not 2)
                    {
                        break;
                    }
                    
                    var type = entry[0].ToString()!;
                    var amount = entry[1].ToInt32()!;
                    character.RandomMissionsFailed[type] = amount;
                    break;
                }
                case "rm_completed":
                {
                    if (entry.Count is not 2)
                    {
                        break;
                    }
                    
                    var type = entry[0].ToString()!;
                    var amount = entry[1].ToInt32()!;
                    character.RandomMissionsCompleted[type] = amount;
                    break;
                }
                case "sys_visited":
                    if (entry.Count is not 0)
                    {
                        character.SystemsVisited.Add(entry[0].ToInt32());
                    }
                    break;
                case "holes_visited":
                    if (entry.Count is not 0)
                    {
                        character.JumpHolesVisited.Add(entry[0].ToInt32());
                    }
                    break;
                case "base_visited":
                    if (entry.Count is not 0)
                    {
                        character.BasesVisited.Add(entry[0].ToInt32());
                    }
                    break;
            }
        }

        if (config.CleanBadData)
        {
            character.BasesVisited = character.BasesVisited.Where(x => Program.GameData.Universe.Bases.FirstOrDefault(y => Program.Hash(y.Nickname) == x) is not null).ToList();
            character.SystemsVisited = character.SystemsVisited.Where(x => Program.GameData.Universe.Systems.FirstOrDefault(y => Program.Hash(y.Nickname) == x) is not null).ToList();
            character.JumpHolesVisited = character.JumpHolesVisited.Where(x => Program.GameData.Universe.Systems.FirstOrDefault(y => y.Objects.FirstOrDefault(z => Program.Hash(z.Nickname) == x) is not null) is not null).ToList();
            character.ShipTypesKilled = character.ShipTypesKilled.Where(x => Program.GameData.Ships.Ships.FirstOrDefault(y => Program.Hash(y.Nickname).ToString() == x.Key) is not null).ToDictionary();
        }

        return true;
    }
}