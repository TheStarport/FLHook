#include "PCH.hpp"

#include "EquipmentEnhancements.hpp"


namespace Plugins
{
    void EquipmentEnhancementsPlugin::ReadMunitionDataFromInis()
    {
        INI_Reader ini;

        char szCurDir[MAX_PATH];
        GetCurrentDirectoryA(sizeof(szCurDir), szCurDir);
        std::string currDir = std::string(szCurDir);
        std::string scFreelancerIniFile = currDir + R"(\freelancer.ini)";

        std::string gameDir = currDir.substr(0, currDir.length() - 4);
        gameDir += std::string(R"(\DATA\)");

        if (!ini.open(scFreelancerIniFile.c_str(), false))
        {
            return;
        }

        std::vector<std::string> equipFiles;
        std::vector<std::string> shipFiles;

        while (ini.read_header())
        {
            if (!ini.is_header("Data"))
            {
                continue;
            }
            while (ini.read_value())
            {
                if (ini.is_value("equipment"))
                {
                    equipFiles.emplace_back(ini.get_value_string());
                }
                else if (ini.is_value("ships"))
                {
                    shipFiles.emplace_back(ini.get_value_string());
                }
            }
        }

        ini.close();

        int maxArmorValue = 0;
        for (std::string shipFile : shipFiles)
        {
            shipFile = gameDir + shipFile;
            if (!ini.open(shipFile.c_str(), false))
            {
                continue;
            }

            Id currNickname;
            ushort currSID = 0;
            while (ini.read_header())
            {
                if (ini.is_header("Ship"))
                {
                    currSID = 3;
                    while (ini.read_value())
                    {
                        if (ini.is_value("nickname"))
                        {
                            currNickname = Id(ini.get_value_string());
                        }
                        else if (ini.is_value("armor"))
                        {
                            int armorValue = ini.get_value_int(0);
                            shipArmorMap[currNickname][1] = armorValue;
                            maxArmorValue = std::max(maxArmorValue, armorValue);
                            break;
                        }
                    }
                }
                else if (ini.is_header("CollisionGroup"))
                {
                    currSID++;
                    while (ini.read_value())
                    {
                        if (ini.is_value("hp_disable"))
                        {
                            shipDataMap[currNickname].colGrpHpMap[currSID].push_back(ini.get_value_string());
                        }
                        else if (ini.is_value("fuse"))
                        {
                            uint fuseHash = CreateID(ini.get_value_string(0));
                            int index = 3;
                            while (true)
                            {
                                std::string HpName = ini.get_value_string(index++);
                                if (HpName.empty())
                                {
                                    break;
                                }
                                shipDataMap[currNickname].fuseHpMap[fuseHash].push_back(HpName);
                            }
                        }
                        else if (ini.is_value("armor"))
                        {
                            int armorValue = ini.get_value_int(0);
                            shipArmorMap[currNickname][currSID] = armorValue;
                            maxArmorValue = std::max(maxArmorValue, armorValue);
                            break;
                        }
                    }
                }
            }

            ini.close();
        }

        armorReductionVector.reserve(maxArmorValue);
        for (int i = 0; i <= maxArmorValue; ++i)
        {
            armorReductionVector.emplace_back(1.0f - (static_cast<float>(i) / (i + ARMOR_MOD)));
        }

        for (std::string& equipFile : equipFiles)
        {
            equipFile = gameDir + equipFile;
            if (!ini.open(equipFile.c_str(), false))
            {
                continue;
            }

            Id currNickname;
            Id explosion_arch;
            while (ini.read_header())
            {
                if (ini.is_header("Gun"))
                {
                    while (ini.read_value())
                    {
                        if (ini.is_value("nickname"))
                        {
                            currNickname = Id(ini.get_value_string(0));
                        }
                        else if (ini.is_value("burst_fire"))
                        {
                            float baseRefire = ((Archetype::Gun*)Archetype::GetEquipment(currNickname.GetValue()))->refireDelay;
                            burstGunData[currNickname] = { ini.get_value_int(0), baseRefire - ini.get_value_float(1) };
                        }
                    }
                }
                else if (ini.is_header("Mine"))
                {
                    while (ini.read_value())
                    {
                        if (ini.is_value("nickname"))
                        {
                            currNickname = Id(ini.get_value_string());
                        }
                        else if (ini.is_value("self_detonate"))
                        {
                            mineInfoMap[currNickname].detonateOnEndLifetime = ini.get_value_bool(0);
                        }
                        else if (ini.is_value("mine_arming_time"))
                        {
                            mineInfoMap[currNickname].armingTime = ini.get_value_float(0);
                        }
                        else if (ini.is_value("stop_spin"))
                        {
                            mineInfoMap[currNickname].stopSpin = ini.get_value_bool(0);
                        }
                        else if (ini.is_value("dispersion_angle"))
                        {
                            mineInfoMap[currNickname].dispersionAngle = ini.get_value_float(0) / (180.f / 3.14f);
                        }
                        else if (ini.is_value("explosion_arch"))
                        {
                            explosion_arch = Id(ini.get_value_string(0));
                        }
                        else if (ini.is_value("detonation_dist"))
                        {
                            explosionTypeMap[explosion_arch].detDist = ini.get_value_float(0);
                        }
                    }
                }
                else if (ini.is_header("Munition"))
                {
                    while (ini.read_value())
                    {
                        if (ini.is_value("nickname"))
                        {
                            currNickname = Id(ini.get_value_string());
                        }
                        else if (ini.is_value("armor_pen"))
                        {
                            weaponDataMap[currNickname].armorPen = ini.get_value_int(0);
                        }
                        else if (ini.is_value("percentage_damage_hull"))
                        {
                            weaponDataMap[currNickname].percentageHullDmg = ini.get_value_float(0);
                        }
                        else if (ini.is_value("percentage_damage_shield"))
                        {
                            weaponDataMap[currNickname].percentageShieldDmg = ini.get_value_float(0);
                        }
                        else if (ini.is_value("percentage_damage_energy"))
                        {
                            weaponDataMap[currNickname].percentageEnergyDmg = ini.get_value_float(0);
                        }
                        else if (ini.is_value("arming_time"))
                        {
                            guidedDataMap[currNickname].armingTime = ini.get_value_float(0);
                        }
                        else if (ini.is_value("no_tracking_alert") && ini.get_value_bool(0))
                        {
                            guidedDataMap[currNickname].noTrackingAlert = true;
                        }
                        else if (ini.is_value("tracking_blacklist"))
                        {
                            uint blacklistedTrackingTypesBitmap = 0;
                            std::string typeStr = StringUtils::ToLower(std::string(ini.get_value_string(0)));
                            if (typeStr.find("fighter") != std::string::npos)
                            {
                                blacklistedTrackingTypesBitmap |= (uint)ObjectType::Fighter;
                            }
                            if (typeStr.find("freighter") != std::string::npos)
                            {
                                blacklistedTrackingTypesBitmap |= (uint)ObjectType::Freighter;
                            }
                            if (typeStr.find("transport") != std::string::npos)
                            {
                                blacklistedTrackingTypesBitmap |= (uint)ObjectType::Transport;
                            }
                            if (typeStr.find("gunboat") != std::string::npos)
                            {
                                blacklistedTrackingTypesBitmap |= (uint)ObjectType::Gunboat;
                            }
                            if (typeStr.find("cruiser") != std::string::npos)
                            {
                                blacklistedTrackingTypesBitmap |= (uint)ObjectType::Cruiser;
                            }
                            if (typeStr.find("capital") != std::string::npos)
                            {
                                blacklistedTrackingTypesBitmap |= (uint)ObjectType::Capital;
                            }
                            if (typeStr.find("guided") != std::string::npos)
                            {
                                blacklistedTrackingTypesBitmap |= (uint)ObjectType::Guided;
                            }
                            if (typeStr.find("mine") != std::string::npos)
                            {
                                blacklistedTrackingTypesBitmap |= (uint)ObjectType::Mine;
                            }

                            guidedDataMap[currNickname].trackingBlacklist = blacklistedTrackingTypesBitmap;
                        }
                        else if (ini.is_value("top_speed"))
                        {
                            guidedDataMap[currNickname].topSpeed = ini.get_value_float(0) * ini.get_value_float(0);
                        }
                        else if (ini.is_value("explosion_arch"))
                        {
                            explosion_arch = Id(ini.get_value_string(0));
                        }
                        else if (ini.is_value("detonation_dist"))
                        {
                            explosionTypeMap[explosion_arch].detDist = ini.get_value_float(0);
                        }
                    }
                }
                else if (ini.is_header("Engine"))
                {
                    EngineProperties ep;
                    bool FoundValue = false;
                    while (ini.read_value())
                    {
                        if (ini.is_value("nickname"))
                        {
                            currNickname = Id(ini.get_value_string());
                        }
                        else if (ini.is_value("disruptor_engine_kill") && !ini.get_value_bool(0))
                        {
                            ep.ignoreCDWhenEKd = true;
                            FoundValue = true;
                        }
                        else if (ini.is_value("disruptor_engine_kill_speed_limit"))
                        {
                            ep.engineKillCDSpeedLimit = ini.get_value_float(0);
                            FoundValue = true;
                        }
                        else if (ini.is_value("hp_type"))
                        {
                            ep.hpType = ini.get_value_string(0);
                            FoundValue = true;
                        }
                    }
                    if (FoundValue)
                    {
                        engineData[currNickname] = ep;
                    }
                }
                else if (ini.is_header("ShieldGenerator"))
                {
                    ShieldBoostData sb;
                    bool FoundValue = false;

                    while (ini.read_value())
                    {
                        if (ini.is_value("nickname"))
                        {
                            currNickname = Id(ini.get_value_string());
                        }
                        else if (ini.is_value("shield_boost"))
                        {
                            sb.durationPerBattery = ini.get_value_float(0);
                            sb.minimumDuration = ini.get_value_float(1);
                            sb.maximumDuration = ini.get_value_float(2);
                            sb.damageReduction = ini.get_value_float(3);
                            sb.fuseId = Id(ini.get_value_string(4));
                            FoundValue = true;
                        }
                        else if (ini.is_value("shield_boost_explosion"))
                        {
                            sb.hullBaseDamage = ini.get_value_float(0);
                            sb.hullReflectDamagePercentage = ini.get_value_float(1);
                            sb.hullDamageCap = ini.get_value_float(2);
                            sb.energyBaseDamage = ini.get_value_float(3);
                            sb.energyReflectDamagePercentage = ini.get_value_float(4);
                            sb.energyDamageCap = ini.get_value_float(5);
                            sb.radius = ini.get_value_float(6);
                            sb.explosionFuse = Id(ini.get_value_string(7));
                        }
                    }
                    if (FoundValue)
                    {
                        shieldBoostMap[currNickname] = sb;
                    }
                }
                else if (ini.is_header("Explosion"))
                {
                    ExplosionDamageData damageType;
                    bool foundItem = false;
                    while (ini.read_value())
                    {
                        if (ini.is_value("nickname"))
                        {
                            currNickname = Id(ini.get_value_string());
                        }
                        else if (ini.is_value("weapon_type"))
                        {
                            damageType.weaponType = Id(ini.get_value_string(0));
                            foundItem = true;
                        }
                        else if (ini.is_value("damage_solars"))
                        {
                            damageType.damageSolars = ini.get_value_bool(0);
                            foundItem = true;
                        }
                        else if (ini.is_value("armor_pen"))
                        {
                            damageType.armorPen = ini.get_value_int(0);
                            foundItem = true;
                        }
                        else if (ini.is_value("percentage_damage_hull"))
                        {
                            damageType.percentageDamageHull = ini.get_value_float(0);
                            foundItem = true;
                        }
                        else if (ini.is_value("percentage_damage_shield"))
                        {
                            damageType.percentageDamageShield = ini.get_value_float(0);
                            foundItem = true;
                        }
                        else if (ini.is_value("percentage_damage_energy"))
                        {
                            damageType.percentageDamageEnergy = ini.get_value_float(0);
                            foundItem = true;
                        }
                        else if (ini.is_value("cruise_disruptor"))
                        {
                            damageType.cruiseDisrupt = ini.get_value_bool(0);
                            foundItem = true;
                        }
                        else if (ini.is_value("destroy_missiles"))
                        {
                            damageType.missileDestroy = ini.get_value_bool(0);
                            foundItem = true;
                        }
                    }
                    if (foundItem)
                    {
                        explosionTypeMap[currNickname] = damageType;
                    }
                }
            }
            ini.close();
        }

        if (ini.open((gameDir + "\\FX\\explosions.ini").c_str(), false))
        {
            while (ini.read_header())
            {
                if (!ini.is_header("explosion"))
                {
                    continue;
                }
                Id currNickname;
                ExplosionDamageData damageType;
                bool foundItem = false;
                while (ini.read_value())
                {
                    if (ini.is_value("nickname"))
                    {
                        currNickname = Id(ini.get_value_string());
                    }
                    else if (ini.is_value("weapon_type"))
                    {
                        damageType.weaponType = Id(ini.get_value_string(0));
                        foundItem = true;
                    }
                    else if (ini.is_value("damage_solars"))
                    {
                        damageType.damageSolars = ini.get_value_bool(0);
                        foundItem = true;
                    }
                    else if (ini.is_value("armor_pen"))
                    {
                        damageType.armorPen = ini.get_value_int(0);
                        foundItem = true;
                    }
                    else if (ini.is_value("percentage_damage_hull"))
                    {
                        damageType.percentageDamageHull = ini.get_value_float(0);
                        foundItem = true;
                    }
                    else if (ini.is_value("percentage_damage_shield"))
                    {
                        damageType.percentageDamageShield = ini.get_value_float(0);
                        foundItem = true;
                    }
                    else if (ini.is_value("percentage_damage_energy"))
                    {
                        damageType.percentageDamageEnergy = ini.get_value_float(0);
                        foundItem = true;
                    }
                    else if (ini.is_value("cruise_disruptor"))
                    {
                        damageType.cruiseDisrupt = ini.get_value_bool(0);
                        foundItem = true;
                    }
                    else if (ini.is_value("destroy_missiles"))
                    {
                        damageType.missileDestroy = ini.get_value_bool(0);
                        foundItem = true;
                    }
                }
                if (foundItem)
                {
                    explosionTypeMap[currNickname] = damageType;
                }
            }
            ini.close();
        }

        for (std::string& shipFile : shipFiles)
        {
            std::string filename = gameDir + shipFile;
            if (!ini.open(filename.c_str(), false))
            {
                continue;
            }

            while (ini.read_header())
            {
                if (!ini.is_header("Ship"))
                {
                    continue;
                }

                std::unordered_set<std::string> shipEngineHPs;
                while (ini.read_value())
                {
                    Id currNickname;
                    if (ini.is_value("nickname"))
                    {
                        currNickname = Id(ini.get_value_string(0));
                        shipEngineHPs.clear();
                    }
                    else if (ini.is_value("equip_override"))
                    {
                        equipOverrideMap[Id(ini.get_value_string(0))][currNickname] = GoodId(CreateID(ini.get_value_string(1)));
                    }
                    else if (ini.is_value("internal_engine"))
                    {
                        shipDataMap[currNickname].internalEngine = ini.get_value_bool(0);
                    }
                    else if (ini.is_value("hp_type"))
                    {
                        std::string equipType = ini.get_value_string(0);
                        int i = 1;
                        while (i < 10)
                        {
                            std::string hardpointName = ini.get_value_string(i);
                            if (hardpointName.empty())
                            {
                                break;
                            }
                            if (hardpointName.find("HpEngine") != std::string::npos)
                            {
                                if (!shipEngineHPs.count(hardpointName))
                                {
                                    shipEngineHPs.insert(hardpointName);
                                    shipDataMap[currNickname].engineCount++;
                                }
                                shipDataMap[currNickname].engineHpMap[equipType].insert(hardpointName);
                            }

                            i++;
                        }
                    }
                }
            }

            ini.close();
        }
    }
} // namespace Plugins
