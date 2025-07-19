#include "PCH.hpp"

#include "EquipmentEnhancements.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/Utils/Random.hpp"
#include <API/FLHook/ResourceManager.hpp>
#include <FLCore/Common/Globals.hpp>

namespace Plugins
{

    void __fastcall PlayerFireRemoveAmmoDetour(PlayerData* pd, void* edx, uint archId, int amount, float hp, bool syncPlayer)
    {
        SrvGun** SrvGunPtr = (SrvGun**)(DWORD(&archId) + 0x14);

        CELauncher* launcher = ((*SrvGunPtr)->launcher);

        typedef void(__thiscall * PlayerFireRemoveAmmo)(PlayerData * pd, uint archId, int amount, float hp, bool syncPlayer);
        static PlayerFireRemoveAmmo PlayerFireRemoveAmmoFunc =
            (PlayerFireRemoveAmmo)(FLHook::Offset(FLHook::BinaryType::Server, AddressList::FireRemoveAmmoFuncCall));

        PlayerFireRemoveAmmoFunc(pd, archId, launcher->GetProjectilesPerFire(), hp, syncPlayer);
    }

    void EquipmentEnhancementsPlugin::MineSpin(CMine* mine, Vector& spinVec)
    {
        auto mineInfo = mineInfoMap.find(mine->archetype->archId);
        if (mineInfo == mineInfoMap.end() || !mineInfo->second.stopSpin)
        {
            PhySys::AddToAngularVelocityOS(mine, spinVec);
        }
    }

    void EquipmentEnhancementsPlugin::MineImpulse(CMine* mine, Vector& launchVec)
    {
        auto mineInfo = mineInfoMap.find(mine->archetype->archId);
        if (mineInfo != mineInfoMap.end() && mineInfo->second.dispersionAngle > 0.0f)
        {
            Vector randVecAxis = RandomVector(1.0f);

            Vector vxp = glm::cross(randVecAxis, launchVec);
            Vector vxvxp = glm::cross(randVecAxis, vxp);

            float angle = mineInfo->second.dispersionAngle;
            angle *= Random::UniformFloat(0.0f, 10000.0f);

            vxp *= sinf(angle);
            vxvxp *= 1.0f - cosf(angle);

            launchVec.x += vxp.x + vxvxp.x;
            launchVec.y += vxp.y + vxvxp.y;
            launchVec.z += vxp.z + vxvxp.z;
        }

        PhySys::AddToVelocity(mine, launchVec);
    }

    float __fastcall EquipmentEnhancementsPlugin::GetWeaponModifier(CEShield* shield, void* edx, Id& weaponType)
    {
        if (!weaponType || !shield || !shield->highestToughnessShieldGenArch)
        {
            return 1.0f;
        }

        auto shieldResistIter = GameData::shieldResistMap.find(weaponType.GetValue());
        if (shieldResistIter == GameData::shieldResistMap.end() || !shieldResistIter.key())
        {
            return 1.0f;
        }

        auto shieldResistMap2 = shieldResistIter.value();
        auto shieldResistIter2 = shieldResistMap2->find(shield->highestToughnessShieldGenArch->shieldTypeId);
        if (shieldResistIter2 == shieldResistMap2->end() || !shieldResistIter2.key())
        {
            return 1.0f;
        }

        return *shieldResistIter2.value();
    }

    void EquipmentEnhancementsPlugin::OnSpRequestUseItem(ClientId client, const SSPUseItem& p1)
    {
        const static uint BATTERY_ARCH_ID = CreateID("ge_s_battery_01");
        usedBatts = false;
        auto ship = client.GetShip();

        if (ship.HasError())
        {
            return;
        }
        const auto& eqManager = ship.Value().GetEquipmentManager().Unwrap();
        const auto& usedItem = reinterpret_cast<const CECargo*>(eqManager->FindByID(p1.itemId));
        if (!usedItem)
        {
            return;
        }
        uint itemArchId = usedItem->archetype->archId.GetValue();
        if (itemArchId == BATTERY_ARCH_ID)
        {
            usedBatts = true;
        }
    }
    void EquipmentEnhancementsPlugin::OnSpRequestUseItemAfter(ClientId client, const SSPUseItem& p1)
    {
        if (!usedBatts)
        {
            return;
        }
        usedBatts = false;

        ShieldState& shieldState = playerShieldState[client.GetValue()];
        shieldState = ShieldState();

        const auto& eqManager = client.GetShip().Unwrap().GetEquipmentManager().Unwrap();
        CEquipTraverser tr((uint)EquipmentClass::ShieldGenerator);
        const CEquip* shield;

        const ShieldBoostData* primaryBoost = nullptr;

        while (shield = eqManager->Traverse(tr))
        {
            const auto& shieldData = shieldBoostMap.find(shield->archetype->archId);
            if (shieldData == shieldBoostMap.end())
            {
                continue;
            }

            primaryBoost = &shieldData->second;
            break;
        }

        if (!primaryBoost)
        {
            return;
        }

        shieldState.damageReduction = std::min(1.0f, primaryBoost->damageReduction);

        const auto& usedItem = reinterpret_cast<const CECargo*>(eqManager->FindByID(p1.itemId));
        int currBattCount = 0;
        if (usedItem)
        {
            currBattCount = usedItem->count;
        }
        uint usedAmount = p1.amountUsed - currBattCount;

        float boostDuration = primaryBoost->durationPerBattery * usedAmount;

        if (boostDuration < primaryBoost->minimumDuration)
        {
            return;
        }

        boostDuration = std::min(primaryBoost->maximumDuration, boostDuration);
        boostDuration *= 1000;

        auto currTime = TimeUtils::UnixTime<std::chrono::milliseconds>();
        if (shieldState.boostUntil && shieldState.boostUntil > currTime)
        {
            shieldState.boostUntil += static_cast<mstime>(boostDuration);
        }
        else
        {
            shieldState.boostUntil = currTime + static_cast<mstime>(boostDuration);
        }

        ShieldBoostFuseInfo& boostInfo = shieldFuseMap[client];
        boostInfo.boostData = primaryBoost;
        boostInfo.lastUntil = shieldState.boostUntil;

        if (!primaryBoost->fuseId)
        {
            return;
        }

        Ship* ship = reinterpret_cast<Ship*>(FLHook::GetObjInspect(client.GetShip().Unwrap().GetId().Unwrap()));

        if (!ship)
        {
            return;
        }

        ship->unlight_fuse_unk(primaryBoost->fuseId, 0, 0.0f);
        ship->light_fuse(0, primaryBoost->fuseId, 0.0f, 0.0f, -1.0f);
    }

    void EquipmentEnhancementsPlugin::OnRequestAddItem(ClientId client, GoodId& goodId, std::wstring_view hardpoint, int count, float status, bool mounted)
    {
        if (mounted && hardpoint == L"BAY")
        {
            auto equip = goodId.GetEquipment().Handle().GetValue();
            if (equip->get_class_type() == Archetype::ClassType::Engine)
            {
                mounted = false;
            }
        }

        auto overrideMapIter = equipOverrideMap.find(goodId.GetHash().Value());
        if (overrideMapIter == equipOverrideMap.end())
        {
            return;
        }

        auto shipOverrideIter = overrideMapIter->second.find(client.GetShipArch().Unwrap().GetId());
        if (shipOverrideIter == overrideMapIter->second.end())
        {
            return;
        }
        goodId = shipOverrideIter->second;
    }

    // Load configuration file
    bool EquipmentEnhancementsPlugin::OnLoadSettings()
    {
        LoadJsonWithValidation(Config, config, "config/equipment_enhancements.json");
        ReadMunitionDataFromInis();

        MemUtils::PatchCallAddr(GetModuleHandleA("Server.dll"), 0xD921, (char*)PlayerFireRemoveAmmoDetour);
        MemUtils::PatchCallAddr(GetModuleHandleA("Common.dll"), 0x4CB81, (char*)MineSpin);
        MemUtils::PatchCallAddr(GetModuleHandleA("Common.dll"), 0x4CAF1, (char*)MineImpulse);

        DWORD GetWeaponAddr = DWORD(&GetWeaponModifier);
        MemUtils::WriteProcMem(DWORD(GetModuleHandleA("Server.dll")) + 0x8426C, &GetWeaponAddr, 4);

        if (!config.shieldExplosionArch.empty())
        {
            ID_String str;
            str.id = CreateID(config.shieldExplosionArch.c_str());
            shieldExplosion = Archetype::GetExplosion(str);
        }

        return true;
    }

    void EquipmentEnhancementsPlugin::OnServerUpdateAfter()
    {
        for (auto iter = topSpeedWatch.begin(); iter != topSpeedWatch.end();)
        {
            auto iGuided = FLHook::GetObjInspect(iter->first);
            if (!iGuided)
            {
                iter = topSpeedWatch.erase(iter);
                continue;
            }

            CGuided* guided = reinterpret_cast<CGuided*>(iGuided->cobj);
            SpeedCheck& speedData = iter->second;

            Vector velocityVec = guided->get_velocity();
            float velocity = glm::length(velocityVec * velocityVec);

            if (velocity > speedData.targetSpeed * 1.02f)
            {
                velocityVec = glm::normalize(velocityVec) * sqrt(speedData.targetSpeed);
                guided->motorData = nullptr;

                const uint physicsPtr = *reinterpret_cast<uint*>(PCHAR(*reinterpret_cast<uint*>(uint(guided) + 84)) + 152);
                Vector* linearVelocity = reinterpret_cast<Vector*>(physicsPtr + 164);
                *linearVelocity = velocityVec;
            }
            iter++;
        }

        for (auto iter = NewMissileForcedUpdatePacketMap.begin(); iter != NewMissileForcedUpdatePacketMap.end();)
        {

            uint counter = ++iter->second;

            if (counter == 1)
            {
                iter++;
                continue;
            }

            Id id = iter->first;
            Guided* guided = nullptr;
            StarSystem* starSystem;
            FLHook::GetObjInspect(id, reinterpret_cast<GameObject*&>(guided), starSystem);

            if (!guided)
            {
                iter = NewMissileForcedUpdatePacketMap.erase(iter);
                continue;
            }

            SSPObjUpdateInfo sp;
            sp.objId = iter->first;
            sp.pos = guided->cobj->position;
            sp.dir = Quaternion(guided->cobj->orientation);
            sp.throttle = 0;
            sp.state = 0;

            for (auto& observer : starSystem->observerList)
            {
                sp.timestamp = static_cast<float>(observer.timestamp);

                FLHook::GetPacketInterface()->Send_FLPACKET_COMMON_UPDATEOBJECT(observer.clientId, sp);
            }

            if (counter >= 3)
            {
                iter = NewMissileForcedUpdatePacketMap.erase(iter);
            }
            else
            {
                iter++;
            }
        }

        if (shieldFuseMap.empty())
        {
            return;
        }

        auto currTime = TimeUtils::UnixTime<std::chrono::milliseconds>();
        static std::vector<ClientId> keysToRemove;
        for (auto& shieldFuse : shieldFuseMap)
        {
            if (shieldFuse.second.lastUntil > currTime)
            {
                continue;
            }

            keysToRemove.emplace_back(shieldFuse.first);
            Ship* ship = nullptr;
            StarSystem* starSystem;
            FLHook::GetObjInspect(shieldFuse.first.GetShip().Handle().GetId().Unwrap(), reinterpret_cast<GameObject*&>(ship), starSystem);
            if (!ship)
            {
                continue;
            }
            ship->unlight_fuse_unk(shieldFuse.second.boostData->fuseId, 0, 0.0f);

            const ShieldBoostData* boostData = shieldFuse.second.boostData;

            if (boostData->radius == 0.0f)
            {
                continue;
            }

            shieldExplosion->radius = boostData->radius;
            shieldExplosion->hullDamage =
                std::min(boostData->hullDamageCap,
                         boostData->hullBaseDamage + boostData->hullReflectDamagePercentage * playerShieldState[shieldFuse.first.GetValue()].damageTaken);
            shieldExplosion->energyDamage =
                std::min(boostData->energyDamageCap,
                         boostData->energyBaseDamage + boostData->energyReflectDamagePercentage * playerShieldState[shieldFuse.first.GetValue()].damageTaken);

            static auto starSystemMap =
                (st6::map<unsigned int, StarSystem, st6::less<unsigned int>, st6::allocator<std::pair<const unsigned int, StarSystem>>>*)0x6D8DA2C;

            auto starSystemIter = starSystemMap->find(shieldFuse.first.GetSystemId().Unwrap().GetValue());
            ExplosionDamageEvent expl;
            expl.attackerId = shieldFuse.first.GetShip().Unwrap().GetId().Unwrap().GetValue();
            expl.projectileId = expl.attackerId;
            expl.dmgCause = DamageCause::Mine;
            expl.explosionPosition = ship->cobj->position;
            expl.explosionArchetype = shieldExplosion;
            expl.dunno = 0;

            typedef void(__thiscall * TriggerExplosion)(StarSystem*, ExplosionDamageEvent*);
            static TriggerExplosion TriggerExplosionFunc = TriggerExplosion(0x6D0B260);

            TriggerExplosionFunc(&starSystemIter->second, &expl);

            if (boostData->explosionFuse)
            {
                ship->unlight_fuse_unk(boostData->explosionFuse, 0, 0.0f);
                ship->light_fuse(0, boostData->explosionFuse, 0, 0.0f, -1.0f);
            }
        }

        for (auto key : keysToRemove)
        {
            shieldFuseMap.erase(key);
        }
        keysToRemove.clear();
    }

    void EquipmentEnhancementsPlugin::OnClearClientInfo(ClientId client) {} // TODO:check }

    bool EquipmentEnhancementsPlugin::VerifyEngines(ClientId client)
    {
        auto& ship = client.GetData().ship;
        auto cship = ship.GetValue().lock();
        if (!cship)
        {
            return true;
        }

        const auto& equip = client.GetEquipCargo().Handle();

        const auto& shipHpDataIter = shipDataMap.find(client.GetShipArch().Handle().GetId());
        if (shipHpDataIter == shipDataMap.end())
        {
            int counter = 0;
            CEquipTraverser tr((uint)EquipmentClass::Engine);
            CEEngine* cequip;
            while (cequip = reinterpret_cast<CEEngine*>(cship->equipManager.Traverse(tr)))
            {
                counter++;
            }
            if (counter == 1)
            {
                return true;
            }
            return false;
        }

        auto& shipHpData = shipHpDataIter->second;

        int mountedEngineCounter = 0;

        bool internalEngineFound = false;

        CEquipTraverser tr((uint)EquipmentClass::Engine);
        CEEngine* cequip;
        while (cequip = reinterpret_cast<CEEngine*>(cship->equipManager.Traverse(tr)))
        {
            auto equipDesc = equip->find_equipment_item(cequip->SubObjId);
            std::string hardpoint = equipDesc->hardPoint.value;

            if (hardpoint == "BAY")
            {
                if (!internalEngineFound && (shipHpData.internalEngine || shipHpData.engineHpMap.empty()))
                {
                    internalEngineFound = true;
                    continue;
                }

                return false;
            }

            mountedEngineCounter++;

            auto engineType = engineData.find(cequip->archetype->archId);
            if (engineType == engineData.end())
            {
                return false;
            }

            auto hpMapIter = shipHpData.engineHpMap.find(engineType->second.hpType);
            if (hpMapIter == shipHpData.engineHpMap.end())
            {
                return false;
            }

            if (!hpMapIter->second.count(hardpoint))
            {
                return false;
            }
        }

        if (mountedEngineCounter != shipHpData.engineCount)
        {
            return false;
        }

        return true;
    }

    void UnmountEngines(ClientId client)
    {
        for (auto& eq : client.GetEquipCargo().Handle()->equip)
        {
            auto equipArch = Archetype::GetEquipment(eq.archId.GetValue());
            if (!equipArch)
            {
                continue;
            }
            if (equipArch->get_class_type() == Archetype::ClassType::Engine)
            {
                eq.mounted = false;
            }
        }
    }

    void EquipmentEnhancementsPlugin::OnPlayerLaunchAfter(const ClientId client, const ShipId& ship)
    {
        playerShieldState[client.GetValue()] = ShieldState();
        shieldFuseMap.erase(client);

        for (auto iter = shieldStateUpdateMap.begin(); iter != shieldStateUpdateMap.end();)
        {
            if (iter->targetClient == client)
            {
                iter = shieldStateUpdateMap.erase(iter);
            }
            else
            {
                ++iter;
            }
        }

        if (!VerifyEngines(client))
        {
            client.Beam(client.GetLastDockedBase().Unwrap());
            UnmountEngines(client);
            client.Kick(L"ERR Invalid engine(s) detected. You will be kicked to unmount the engines.", 5);
        }

        equipUpdateVector.push_back({ client, 0 });
    }

    void EquipmentEnhancementsPlugin::OnCShipInitAfter(CShip* ship)
    {
        if (ship->ownerPlayer)
        {
            return;
        }

        CEquipTraverser tr((uint)EquipmentClass::Gun);
        CELauncher* gun;
        while (gun = reinterpret_cast<CELauncher*>(ship->equipManager.Traverse(tr)))
        {
            auto burstGunDataIter = burstGunData.find(gun->archetype->archId);
            if (burstGunDataIter == burstGunData.end())
            {
                continue;
            }

            shipGunData[ship->id][gun->SubObjId] = { burstGunDataIter->second.magSize,
                                                                burstGunDataIter->second.magSize,
                                                                burstGunDataIter->second.reloadTime };
        }
    }

    void EquipmentEnhancementsPlugin::OnCELauncherFireAfter(CELauncher* launcher, const Vector& pos, FireResult fireResult)
    {
        if (fireResult != FireResult::Success && launcher->owner->ownerPlayer && launcher->owner->objectClass != CObject::CSHIP_OBJECT)
        {
            return;
        }

        auto shipDataIter = shipGunData.find(launcher->owner->id);
        if (shipDataIter == shipGunData.end())
        {
            return;
        }

        auto gunData = shipDataIter->second.find(launcher->SubObjId);
        if (gunData == shipDataIter->second.end())
        {
            return;
        }

        if (--gunData->second.bulletsLeft == 0)
        {
            gunData->second.bulletsLeft = gunData->second.maxMagSize;
            launcher->refireDelayElapsed = gunData->second.reloadTime;
        }
    }

    void EquipmentEnhancementsPlugin::OnShipDespawn(Ship* ship) { shipGunData.erase(Id(ship->get_id())); }

    void EquipmentEnhancementsPlugin::OnShipDestroy(Ship* ship, DamageList* dmgList, ShipId killerId) { shipGunData.erase(Id(ship->get_id())); }

    void EquipmentEnhancementsPlugin::OnMineDestroy(Mine* mine, DestroyType& destroyType, ShipId killerId)
    {
        CMine* cmine = reinterpret_cast<CMine*>(mine->cmine());
        auto mineArch = cmine->minearch();

        auto mineInfo = mineInfoMap.find(mineArch->archId);
        if (mineInfo == mineInfoMap.end())
        {
            return;
        }

        if (destroyType == DestroyType::Fuse && mineArch->lifeTime - cmine->remainingLifetime < mineInfo->second.armingTime)
        {
            destroyType = DestroyType::Vanish;
            return;
        }

        if (destroyType == DestroyType::Vanish && mineInfo->second.detonateOnEndLifetime)
        {
            destroyType = DestroyType::Fuse;
        }
    }

    void EquipmentEnhancementsPlugin::OnGuidedDestroy(Guided* guided, DestroyType& destroyType, ShipId killerId)
    {
        NewMissileForcedUpdatePacketMap.erase(Id(guided->get_id()));

        auto guidedInfo = guidedDataMap.find(guided->cobj->archetype->archId);
        if (guidedInfo == guidedDataMap.end())
        {
            return;
        }

        if (guidedInfo->second.armingTime)
        {
            float armingTime = guidedInfo->second.armingTime;
            if (guided->cguided()->lifetime < armingTime)
            {
                destroyType = DestroyType::Vanish;
            }
        }
    }

    void EquipmentEnhancementsPlugin::OnShipMunitionHit(Ship* ship, MunitionImpactData* impact, DamageList* dmgList)
    { 
        FetchShipArmor(ship->cobj->archetype->archId);
        FetchWeaponData(impact->munitionArch->archId);
        armorEnabled = true;
    }

    void EquipmentEnhancementsPlugin::OnShipMunitionHitAfter(Ship* ship, MunitionImpactData* impact, DamageList* dmgList) { armorEnabled = false; }

    void EquipmentEnhancementsPlugin::OnShipEquipDmg(Ship* ship, CAttachedEquip* equip, float& incDmg, DamageList* dmg)
    {

        if (!ship->is_player())
        {
            return;
        }

        auto cship = ship->cship();
        auto invulData = invulMap.find(cship->id.GetValue());
        if (invulData == invulMap.end() || invulData->second.invulType == InvulType::HULLONLY)
        {
            return;
        }
        float minHpAllowed = invulData->second.minHpPerc * equip->archetype->hitPoints;
        if (equip->hitPts <= minHpAllowed)
        {
            return;
        }

        float hpPercPostDamage = equip->hitPts - incDmg;
        incDmg = std::min(incDmg, equip->hitPts - minHpAllowed);
    }

    void EquipmentEnhancementsPlugin::OnShipEquipDestroy(Ship* ship, CEquip* equip, DamageEntry::SubObjFate fate, DamageList* dmg)
    {
        if (equip->CEquipType != EquipmentClass::ShieldGenerator)
        {
            return;
        }

        CEShield* shield = reinterpret_cast<CEShield*>(ship->cship()->equipManager.FindFirst((uint)EquipmentClass::Shield));
        if (!shield)
        {
            return;
        }

        if (shield->linkedShieldGen.size() == 1)
        {
            ship->cequip_death(shield, fate, dmg);
        }
    }

    void EquipmentEnhancementsPlugin::OnShipExplosionHit(Ship* ship, ExplosionDamageEvent* explosion, DamageList* dmgList)
    {
        FetchShipArmor(ship->cobj->archetype->archId);
        FetchWeaponData(explosion->explosionArchetype->id);
        if (!config.hitRayMissileLogic)
        {
            return;
        }
        returnCode = ReturnCode::SkipFunctionCall;
        EqObjExplosionHit(ship, explosion, dmgList);
    }

    void EquipmentEnhancementsPlugin::OnSolarExplosionHit(Solar* solar, ExplosionDamageEvent* explosion, DamageList* dmgList)
    {
        if (!config.hitRayMissileLogic)
        {
            return;
        }
        returnCode = ReturnCode::SkipFunctionCall;
        EqObjExplosionHit(solar, explosion, dmgList);
    }

    void EquipmentEnhancementsPlugin::OnShipShieldDmg(Ship* ship, CEShield* shield, float& incDmg, DamageList* dmgList)
    {
        if (armorEnabled && currMunitionData)
        {
            incDmg += currMunitionData->percentageShieldDmg * shield->maxShieldHitPoints;
        }
    }

    void EquipmentEnhancementsPlugin::OnShipEnergyDmg(Ship* ship, float& incDmg, DamageList* dmgList)
    {
        if (armorEnabled && currMunitionData)
        {
            incDmg += currMunitionData->percentageEnergyDmg * ship->cship()->maxPower;
        }
    }

    void EquipmentEnhancementsPlugin::FetchWeaponData(Id munitionArchId)
    {
        if (currMunitionArch == munitionArchId)
        {
            return;
        }
        currMunitionArch = munitionArchId;

        auto weaponData = weaponDataMap.find(currMunitionArch);
        if (weaponData == weaponDataMap.end())
        {
            currMunitionData = nullptr;
        }
        else
        {
            currMunitionData = &weaponData->second;
        }

    }

    void EquipmentEnhancementsPlugin::FetchShipArmor(Id shipHash)
    {
        if (shipArmorArch == shipHash)
        {
            return;
        }

        shipArmorIter = shipArmorMap.find(shipHash);
        shipArmorArch = shipHash;
        if (shipArmorIter == shipArmorMap.end())
        {
            shipArmorRating = 0;
        }
        else
        {
            shipArmorRating = shipArmorIter->second[1];
        }
    }

    void EquipmentEnhancementsPlugin::OnShipHullDmg(Ship* ship, float& incDmg, DamageList* dmg)
    {
        if (armorEnabled)
        {
            if (currMunitionData && currMunitionData->percentageHullDmg)
            {
                incDmg += ship->cobj->archetype->hitPoints * currMunitionData->percentageHullDmg;
            }

            if (shipArmorRating && currMunitionData)
            {
                incDmg *= armorReductionVector.at(std::max(0, shipArmorRating - currMunitionData->armorPen));
            }

            armorEnabled = false;
        }

        if (ship->is_player())
        {
            auto cship = ship->cship();
            auto invulIter = invulMap.find(cship->id.GetValue());
            if (invulIter == invulMap.end() || invulIter->second.invulType == InvulType::EQUIPONLY)
            {
                return;
            }

            float minHpAllowed = invulIter->second.minHpPerc * cship->archetype->hitPoints;
            if (cship->hitPoints <= minHpAllowed)
            {
                incDmg = 0;
                return;
            }

            float hpPercPostDamage = cship->hitPoints - incDmg;
            incDmg = std::min(incDmg, cship->hitPoints - minHpAllowed);
        }
    }

    void EquipmentEnhancementsPlugin::OnCGuidedInitAfter(CGuided* guided)
    {
        NewMissileForcedUpdatePacketMap[guided->id] = { 0 };

        // If missile target doesnt match with the ship target, remove tracking
        GameObject* owner = FLHook::GetObjInspect(guided->ownerId);
        IObjRW* ownerTarget = nullptr;
        if (owner)
        {
            ownerTarget = nullptr;
            owner->get_target(ownerTarget);
            if (!ownerTarget)
            {
                guided->set_target(nullptr);
                guided->set_sub_target(0);
            }
        }

        auto guidedData = guidedDataMap.find(guided->archetype->archId);
        if (guidedData == guidedDataMap.end())
        {
            return;
        }

        auto& guidedInfo = guidedData->second;

        if (guidedInfo.trackingBlacklist && ownerTarget)
        {
            auto cobj = reinterpret_cast<CSimple*>(ownerTarget->cobject());
            if (cobj->objectClass & CObject::CSIMPLE_MASK && (uint)cobj->type & guidedInfo.trackingBlacklist)
            {
                guided->set_target(nullptr);
                guided->set_sub_target(0);
            }
        }

        if (guidedInfo.topSpeed)
        {
            topSpeedWatch[guided->id.GetValue()] = { guidedInfo.topSpeed, 0 };
        }
    }

    EquipmentEnhancementsPlugin::EquipmentEnhancementsPlugin(const PluginInfo& info) : Plugin(info) {}

} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
    return PluginInfo{
        .name = L"Equipment Enhancements",
        .shortName = L"equipment_enhancements",
        .versionMajor = PluginMajorVersion::V05,
        .versionMinor = PluginMinorVersion::V00
    };
};

SetupPlugin(EquipmentEnhancementsPlugin);
