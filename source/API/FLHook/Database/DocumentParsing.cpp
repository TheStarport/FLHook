#include "PCH.hpp"

#include "API/FLHook/AccountManager.hpp"





Account::Account(const B_VIEW view)
{
    for (auto& element : view)
    {
        switch (Hash(element.key().data()))
        {
            case Hash("_id"): _id = element.get_string(); break;
            case Hash("characters"):
                {
                    for (auto id : element.get_array().value)
                    {
                        characters.emplace_back(id.get_oid().value);
                    }
                    break;
                }
            case Hash("banned"):
                {
                    banned = element.get_bool();
                    break;
                }
            case Hash("cash"):
                {
                    cash = element.get_int64();
                    break;
                }
            case Hash("scheduledUnbanDate"):
                {
                    scheduledUnbanDate = element.get_int64();
                    break;
                }
            case Hash("gameRoles"):
                {
                    gameRoles = std::vector<std::string>{};
                    for (auto id : element.get_array().value)
                    {
                        gameRoles->emplace_back(id.get_string().value);
                    }
                    break;
                }
            case Hash("webRoles"):
                {
                    webRoles = std::vector<std::string>{};
                    for (auto id : element.get_array().value)
                    {
                        webRoles->emplace_back(id.get_string().value);
                    }
                    break;
                }
            case Hash("hashedToken"):
                {
                    hashedToken = element.get_string();
                    break;
                }
            case Hash("username"):
                {
                    username = element.get_string();
                    break;
                }
            case Hash("passwordHash"):
                {
                    passwordHash = element.get_string();
                    break;
                }
            case Hash("salt"):
                {
                    salt = std::vector<byte>{};
                    auto [subType, size, bytes] = element.get_binary();
                    for (auto i = 0; i < size; ++i)
                    {
                        salt->emplace_back(bytes[i]);
                    }
                    break;
                }
        }
    }
}

void Account::ToBson(B_DOC& document) const
{
    document.append(B_KVP("_id", _id));
    document.append(B_KVP("banned", banned));
    document.append(B_KVP("cash", cash));

    {
        B_ARR arr;
        for (auto& character : characters)
        {
            arr.append(character);
        }
        document.append(B_KVP("characters", arr));
    }

    if (scheduledUnbanDate.has_value())
    {
        document.append(B_KVP("scheduledUnbanDate", scheduledUnbanDate.value()));
    }

    if (gameRoles.has_value())
    {
        B_ARR arr;
        for (auto& role : gameRoles.value())
        {
            arr.append(role);
        }
        document.append(B_KVP("gameRoles", arr));
    }

    if (webRoles.has_value())
    {
        B_ARR arr;
        for (auto& role : webRoles.value())
        {
            arr.append(role);
        }
        document.append(B_KVP("webRoles", arr));
    }

    if (hashedToken.has_value())
    {
        document.append(B_KVP("hashedToken", hashedToken.value()));
    }

    if (username.has_value())
    {
        document.append(B_KVP("username", username.value()));
    }

    if (passwordHash.has_value())
    {
        document.append(B_KVP("passwordHash", passwordHash.value()));
    }

    if (salt.has_value())
    {
        bsoncxx::types::b_binary data{ bsoncxx::binary_sub_type::k_binary, salt->size(), salt->data() };
        document.append(B_KVP("salt", data));
    }
}

Character::Character(B_VIEW view)
{
    for (auto& element : view)
    {
        switch (Hash(element.key().data()))
        {
            case Hash("_id"):
                {
                    _id = element.get_oid().value;
                    break;
                }
            case Hash("accountId"):
                {
                    accountId = element.get_string().value;
                    break;
                }
            case Hash("characterName"):
                {
                    characterName = element.get_string().value;
                    wideCharacterName = StringUtils::stows(characterName);
                    break;
                }
            case Hash("money"):
                {
                    money = element.get_int32().value;
                    break;
                }
            case Hash("rank"):
                {
                    rank = element.get_int32().value;
                    break;
                }
            case Hash("affiliation"):
                {
                    affiliation = element.get_int32().value;
                    break;
                }
            case Hash("repGroup"):
                {
                    repGroup = element.get_string().value;
                    break;
                }
            case Hash("pos"):
                {
                    int counter = 0;
                    for (auto var : element.get_array().value)
                    {
                        pos[counter++] = static_cast<float>(var.get_double().value);
                    }
                    break;
                }
            case Hash("rot"):
                {
                    int counter = 0;
                    for (auto var : element.get_array().value)
                    {
                        rot[counter++] = static_cast<float>(var.get_double().value);
                    }
                    break;
                }
            case Hash("voice"):
                {
                    voice = element.get_string().value;
                    break;
                }
            case Hash("interfaceState"):
                {
                    interfaceState = element.get_int32().value;
                    break;
                }
            case Hash("hullStatus"):
                {
                    hullStatus = static_cast<float>(element.get_double().value);
                    break;
                }
            case Hash("baseHullStatus"):
                {
                    baseHullStatus = static_cast<float>(element.get_double().value);
                    break;
                }
            case Hash("canDock"):
                {
                    canDock = element.get_bool().value;
                    break;
                }
            case Hash("canTradeLane"):
                {
                    canTradeLane = element.get_bool().value;
                    break;
                }
            case Hash("tlExceptions"):
                {
                    tlExceptions = std::vector<TradeLaneException>{};
                    for (auto& tl : element.get_array().value)
                    {
                        auto doc = tl.get_document().value;
                        TradeLaneException exception{};

                        for (auto el : doc)
                        {
                            if (std::string_view key = el.key(); key == "startRingId")
                            {
                                exception.startRingId = el.get_int32().value;
                            }
                            else if (key == "nextRingId")
                            {
                                exception.nextRingId = el.get_int32().value;
                            }
                        }

                        tlExceptions->emplace_back(exception);
                    }
                    break;
                }
            case Hash("dockExceptions"):
                {
                    dockExceptions = std::vector<int>{};
                    for (auto baseId : element.get_array().value)
                    {
                        dockExceptions->emplace_back(baseId.get_int32().value);
                    }
                    break;
                }
            case Hash("lastDockedBase"):
                {
                    lastDockedBase = element.get_int32().value;
                    break;
                }
            case Hash("currentBase"):
                {
                    currentBase = element.get_int32().value;
                    break;
                }
            case Hash("currentRoom"):
                {
                    currentRoom = element.get_int32().value;
                    break;
                }
            case Hash("killCount"):
                {
                    killCount = element.get_int32().value;
                    break;
                }
            case Hash("missionFailureCount"):
                {
                    missionFailureCount = element.get_int32().value;
                    break;
                }
            case Hash("missionSuccessCount"):
                {
                    missionSuccessCount = element.get_int32().value;
                    break;
                }
            case Hash("shipHash"):
                {
                    shipHash = element.get_int32().value;
                    break;
                }
            case Hash("system"):
                {
                    system = element.get_int32().value;
                    break;
                }
            case Hash("totalTimePlayed"):
                {
                    totalTimePlayed = static_cast<float>(element.get_double().value);
                    break;
                }
            case Hash("totalCashEarned"):
                {
                    totalCashEarned = static_cast<float>(element.get_double().value);
                    break;
                }
            case Hash("baseCostume"):
                {
                    for (auto& el : element.get_document().value)
                    {
                        if (std::string_view key = el.key(); key == "head")
                        {
                            baseCostume.head = static_cast<uint>(el.get_int32().value);
                        }
                        else if (key == "body")
                        {
                            baseCostume.body = static_cast<uint>(el.get_int32().value);
                        }
                        else if (key == "leftHand")
                        {
                            baseCostume.leftHand = static_cast<uint>(el.get_int32().value);
                        }
                        else if (key == "rightHand")
                        {
                            baseCostume.rightHand = static_cast<uint>(el.get_int32().value);
                        }
                        else if (key == "accessory")
                        {
                            int counter = 0;
                            for (auto acc : el.get_array().value)
                            {
                                baseCostume.accessory[counter++] = static_cast<uint>(acc.get_int32().value);
                            }
                        }
                        else if (key == "accessories")
                        {
                            baseCostume.accessories = el.get_int32().value;
                        }
                    }
                    break;
                }
            case Hash("commCostume"):
                {
                    for (auto& el : element.get_document().value)
                    {
                        if (std::string_view key = el.key(); key == "head")
                        {
                            commCostume.head = static_cast<uint>(el.get_int32().value);
                        }
                        else if (key == "body")
                        {
                            commCostume.body = static_cast<uint>(el.get_int32().value);
                        }
                        else if (key == "leftHand")
                        {
                            commCostume.leftHand = static_cast<uint>(el.get_int32().value);
                        }
                        else if (key == "rightHand")
                        {
                            commCostume.rightHand = static_cast<uint>(el.get_int32().value);
                        }
                        else if (key == "accessory")
                        {
                            int counter = 0;
                            for (auto acc : el.get_array().value)
                            {
                                commCostume.accessory[counter++] = static_cast<uint>(acc.get_int32().value);
                            }
                        }
                        else if (key == "accessories")
                        {
                            commCostume.accessories = el.get_int32().value;
                        }
                    }
                    break;
                }
            case Hash("cargo"):
                {
                    cargo = std::vector<FLCargo>{};
                    for (auto& item : element.get_array().value)
                    {
                        auto doc = item.get_document().value;
                        FLCargo cargoItem{};

                        for (auto el : doc)
                        {
                            if (std::string_view key = el.key(); key == "archId")
                            {
                                cargoItem.archId = el.get_int32().value;
                            }
                            else if (key == "amount")
                            {
                                cargoItem.amount = static_cast<ushort>(el.get_int32().value);
                            }
                            else if (key == "health")
                            {
                                cargoItem.health = static_cast<float>(el.get_double().value);
                            }
                            else if (key == "isMissionCargo")
                            {
                                cargoItem.isMissionCargo = el.get_bool().value;
                            }
                        }

                        cargo.emplace_back(cargoItem);
                    }
                    break;
                }
            case Hash("baseCargo"):
                {
                    baseCargo = std::vector<FLCargo>{};
                    for (auto& item : element.get_array().value)
                    {
                        auto doc = item.get_document().value;
                        FLCargo cargoItem{};

                        for (auto el : doc)
                        {
                            if (std::string_view key = el.key(); key == "archId")
                            {
                                cargoItem.archId = el.get_int32().value;
                            }
                            else if (key == "amount")
                            {
                                cargoItem.amount = static_cast<ushort>(el.get_int32().value);
                            }
                            else if (key == "health")
                            {
                                cargoItem.health = static_cast<float>(el.get_double().value);
                            }
                            else if (key == "isMissionCargo")
                            {
                                cargoItem.isMissionCargo = el.get_bool().value;
                            }
                        }

                        baseCargo.emplace_back(cargoItem);
                    }
                    break;
                }
            case Hash("equipment"):
                {
                    equipment = std::vector<Equipment>{};
                    for (auto& item : element.get_array().value)
                    {
                        auto doc = item.get_document().value;
                        Equipment eq;

                        for (auto el : doc)
                        {
                            if (std::string_view key = el.key(); key == "archId")
                            {
                                eq.archId = el.get_int32().value;
                            }
                            else if (key == "hardPoint")
                            {
                                eq.hardPoint = el.get_string().value;
                            }
                            else if (key == "health")
                            {
                                eq.health = static_cast<float>(el.get_double().value);
                            }
                            else if (key == "mounted")
                            {
                                eq.mounted = el.get_bool().value;
                            }
                            else if (key == "amount")
                            {
                                eq.amount = static_cast<short>(el.get_int32().value);
                            }
                        }

                        equipment.emplace_back(eq);
                    }
                    break;
                }
            case Hash("baseEquipment"):
                {
                    baseEquipment = std::vector<Equipment>{};
                    for (auto& item : element.get_array().value)
                    {
                        auto doc = item.get_document().value;
                        Equipment eq;

                        for (auto el : doc)
                        {
                            if (std::string_view key = el.key(); key == "archId")
                            {
                                eq.archId = el.get_int32().value;
                            }
                            else if (key == "hardpoint")
                            {
                                eq.hardPoint = el.get_string().value;
                            }
                            else if (key == "health")
                            {
                                eq.health = static_cast<float>(el.get_double().value);
                            }
                            else if (key == "mounted")
                            {
                                eq.mounted = el.get_bool().value;
                            }
                            else if (key == "amount")
                            {
                                eq.amount = static_cast<short>(el.get_int32().value);
                            }
                        }

                        baseEquipment.emplace_back(eq);
                    }
                    break;
                }
            case Hash("collisionGroups"):
                {
                    for (auto& el : element.get_document().value)
                    {
                        collisionGroups[std::string(el.key())] = static_cast<float>(el.get_double().value);
                    }
                    break;
                }
            case Hash("baseCollisionGroups"):
                {
                    for (auto& el : element.get_document().value)
                    {
                        baseCollisionGroups[std::string(el.key())] = static_cast<float>(el.get_double().value);
                    }
                    break;
                }
            case Hash("reputation"):
                {
                    for (auto& el : element.get_document().value)
                    {
                        reputation[std::string(el.key())] = static_cast<float>(el.get_double().value);
                    }
                    break;
                }
            case Hash("shipTypesKilled"):
                {
                    for (auto el : element.get_document().value)
                    {
                        shipTypesKilled[std::string(el.key())] = el.get_int32().value;
                    }
                    break;
                }
            case Hash("randomMissionsCompleted"):
                {
                    for (auto& el : element.get_document().value)
                    {
                        randomMissionsCompleted[std::string(el.key())] = el.get_int32().value;
                    }
                    break;
                }
            case Hash("randomMissionsAborted"):
                {
                    for (auto& el : element.get_document().value)
                    {
                        randomMissionsAborted[std::string(el.key())] = el.get_int32().value;
                    }
                    break;
                }
            case Hash("randomMissionsFailed"):
                {
                    for (auto& el : element.get_document().value)
                    {
                        randomMissionsFailed[std::string(el.key())] = el.get_int32().value;
                    }
                    break;
                }
            case Hash("visits"):
                {
                    for (auto& el : element.get_array().value)
                    {
                        auto array = el.get_array().value;
                        visits.emplace_back( std::array{array[0].get_int32().value, array[1].get_int32().value});
                    }
                    break;
                }
            case Hash("systemsVisited"):
                {
                    systemsVisited = std::vector<int>{};
                    for (auto& baseId : element.get_array().value)
                    {
                        systemsVisited.emplace_back(baseId.get_int32().value);
                    }
                    break;
                }
            case Hash("basesVisited"):
                {
                    basesVisited = std::vector<int>{};
                    for (auto& baseId : element.get_array().value)
                    {
                        basesVisited.emplace_back(baseId.get_int32().value);
                    }
                    break;
                }
            case Hash("npcVisits"):
                {
                    npcVisits = std::vector<NpcVisit>{};
                    for (auto& vnpc : element.get_array().value)
                    {
                        auto doc = vnpc.get_document().value;
                        NpcVisit visit{};

                        for (auto& el : doc)
                        {
                            if (std::string_view key = el.key(); key == "id")
                            {
                                visit.id = el.get_int32().value;
                            }
                            else if (key == "baseId")
                            {
                                visit.baseId = el.get_int32().value;
                            }
                            else if (key == "interactionCount")
                            {
                                visit.interactionCount = el.get_int32().value;
                            }
                            else if (key == "missionStatus")
                            {
                                visit.missionStatus = el.get_int32().value;
                            }
                        }

                        npcVisits.emplace_back(visit);
                    }
                    break;
                }
            case Hash("jumpHolesVisited"):
                {
                    jumpHolesVisited = std::vector<int>{};
                    for (auto& jumpHoleId : element.get_array().value)
                    {
                        jumpHolesVisited.emplace_back(jumpHoleId.get_int32().value);
                    }
                    break;
                }
            case Hash("rumorsReceived"):
                {
                    rumorsReceived = std::vector<RumorData>{};
                    for (auto& rumorData : element.get_array().value)
                    {
                        auto doc = rumorData.get_document().value;
                        RumorData rumor{};

                        for (auto el : doc)
                        {
                            if (std::string_view key = el.key(); key == "rumorIds")
                            {
                                rumor.rumorIds = el.get_int32().value;
                            }
                            else if (key == "priority")
                            {
                                rumor.priority = el.get_int32().value;
                            }
                        }

                        rumorsReceived.emplace_back(rumor);
                    }
                    break;
                }
            case Hash("weaponGroups"):
                {
                    for (auto el : element.get_document().value)
                    {
                        auto group = weaponGroups[std::string(el.key())] = {};
                        for (auto weapon : el.get_array().value)
                        {
                            group.emplace_back(weapon.get_string().value);
                        }
                    }
                    break;
                }
            case Hash("lastRenameTimestamp"):
                {
                    lastRenameTimestamp = element.get_date().to_int64();
                    break;
                }
            case Hash("characterTransferCode"):
                {
                    characterTransferCode = element.get_string().value;
                    break;
                }
            case Hash("presetMsgs"):
                {
                    presetMsgs = std::array<std::string, 10>{};
                    uint i = 0;
                    for (auto& message : element.get_array().value)
                    {
                        presetMsgs.at(i) = message.get_string().value;
                        i++;
                    }
                    break;
                }
            default:
                break;
        }
    }

    HandleDeadCharacter();
}

void Character::HandleDeadCharacter()
{
    // They died, fallback to base data
    if (hullStatus <= 0.0f)
    {
        hullStatus = baseHullStatus;
        cargo = baseCargo;
        equipment = baseEquipment;
        collisionGroups = baseCollisionGroups;
        currentBase = lastDockedBase;
        system = Universe::get_base(currentBase)->systemId.GetValue();
    }
}

void Character::ToBson(B_DOC& document)
{
    HandleDeadCharacter();

    if (_id.has_value())
    {
        document.append(B_KVP("_id", _id.value()));
    }

    document.append(B_KVP("accountId", accountId));
    document.append(B_KVP("characterName", characterName));
    document.append(B_KVP("money", money));
    document.append(B_KVP("rank", rank));
    document.append(B_KVP("affiliation", affiliation));

    if (repGroup.has_value())
    {
        document.append(B_KVP("repGroup", repGroup.value()));
    }

    document.append(B_KVP("pos", B_MARR(pos.x, pos.y, pos.z)));
    document.append(B_KVP("rot", B_MARR(rot.x, rot.y, rot.z)));
    document.append(B_KVP("voice", voice));
    document.append(B_KVP("interfaceState", interfaceState));
    document.append(B_KVP("hullStatus", hullStatus));
    document.append(B_KVP("baseHullStatus", baseHullStatus));
    document.append(B_KVP("canDock", canDock));
    document.append(B_KVP("canTradeLane", canTradeLane));

    if (tlExceptions.has_value())
    {
        auto arr = B_ARR{};
        for (auto& [startRingId, nextRingId] : tlExceptions.value())
        {
            arr.append(B_MDOC(B_KVP("startRingId", startRingId), B_KVP("nextRingId", nextRingId)));
        }
        document.append(B_KVP("tlExceptions", arr));
    }

    if (dockExceptions.has_value())
    {
        auto arr = B_ARR{};
        for (auto& base : dockExceptions.value())
        {
            arr.append(base);
        }
        document.append(B_KVP("dockExceptions", arr));
    }
    document.append(B_KVP("lastDockedBase", lastDockedBase));
    document.append(B_KVP("currentBase", currentBase));
    document.append(B_KVP("currentRoom", currentRoom));
    document.append(B_KVP("killCount", killCount));
    document.append(B_KVP("missionFailureCount", missionFailureCount));
    document.append(B_KVP("missionSuccessCount", missionSuccessCount));
    document.append(B_KVP("shipHash", shipHash));
    document.append(B_KVP("system", system));
    document.append(B_KVP("totalTimePlayed", totalTimePlayed));
    document.append(B_KVP("totalCashEarned", totalCashEarned));
    // clang-format off
    document.append(B_KVP("baseCostume", B_MDOC(
        B_KVP("head", static_cast<int>(baseCostume.head)),
        B_KVP("body", static_cast<int>(baseCostume.body)),
        B_KVP("leftHand", static_cast<int>(baseCostume.leftHand)),
        B_KVP("rightHand", static_cast<int>(baseCostume.rightHand)),
        B_KVP("accessories", static_cast<int>(baseCostume.accessories)),
        B_KVP("accessory", B_MARR(
            static_cast<int>(baseCostume.accessory[0]),
            static_cast<int>(baseCostume.accessory[1]),
            static_cast<int>(baseCostume.accessory[2]),
            static_cast<int>(baseCostume.accessory[3]),
            static_cast<int>(baseCostume.accessory[4]),
            static_cast<int>(baseCostume.accessory[5]),
            static_cast<int>(baseCostume.accessory[6]),
            static_cast<int>(baseCostume.accessory[7])
            ))
        )));

    document.append(B_KVP("commCostume", B_MDOC(
        B_KVP("head", static_cast<int>(commCostume.head)),
        B_KVP("body", static_cast<int>(commCostume.body)),
        B_KVP("leftHand", static_cast<int>(commCostume.leftHand)),
        B_KVP("rightHand", static_cast<int>(commCostume.rightHand)),
        B_KVP("accessories", static_cast<int>(commCostume.accessories)),
        B_KVP("accessory", B_MARR(
            static_cast<int>(commCostume.accessory[0]),
            static_cast<int>(commCostume.accessory[1]),
            static_cast<int>(commCostume.accessory[2]),
            static_cast<int>(commCostume.accessory[3]),
            static_cast<int>(commCostume.accessory[4]),
            static_cast<int>(commCostume.accessory[5]),
            static_cast<int>(commCostume.accessory[6]),
            static_cast<int>(commCostume.accessory[7])
            ))
        )));
    // clang-format on

    {
        B_ARR arr;
        for (auto& flcargo : cargo)
        {
            arr.append(flcargo.ToBson());
        }
        document.append(B_KVP("cargo", arr));
    }

    {
        B_ARR arr;
        for (const auto& [archId, amount, health, isMissionCargo] : baseCargo)
        {
            arr.append(B_MDOC(B_KVP("archId", archId), B_KVP("amount", amount), B_KVP("health", health), B_KVP("isMissionCargo", isMissionCargo)));
        }
        document.append(B_KVP("baseCargo", arr));
    }

    {
        B_ARR arr;
        for (const auto& [archId, hardPoint, health, amount, mounted] : equipment)
        {
            arr.append(
                B_MDOC(B_KVP("archId", archId), B_KVP("hardPoint", hardPoint), B_KVP("health", health), B_KVP("mounted", mounted), B_KVP("amount", amount)));
        }
        document.append(B_KVP("equipment", arr));
    }

    {
        B_ARR arr;
        for (const auto& [archId, hardPoint, health, amount, mounted] : baseEquipment)
        {
            arr.append(
                B_MDOC(B_KVP("archId", archId), B_KVP("hardPoint", hardPoint), B_KVP("health", health), B_KVP("mounted", mounted), B_KVP("amount", amount)));
        }
        document.append(B_KVP("baseEquipment", arr));
    }

    {
        B_DOC doc;
        for (auto& [id, health] : collisionGroups)
        {
            doc.append(B_KVP(id, health));
        }
        document.append(B_KVP("collisionGroups", doc));
    }

    {
        B_DOC doc;
        for (auto& [id, health] : baseCollisionGroups)
        {
            doc.append(B_KVP(id, health));
        }
        document.append(B_KVP("baseCollisionGroups", doc));
    }

    {
        B_DOC doc;
        for (auto& [faction, rep] : reputation)
        {
            doc.append(B_KVP(faction, rep));
        }
        document.append(B_KVP("reputation", doc));
    }

    {
        B_DOC doc;
        for (auto& [shipArch, amount] : shipTypesKilled)
        {
            doc.append(B_KVP(shipArch, amount));
        }
        document.append(B_KVP("shipTypesKilled", doc));
    }

    {
        B_DOC doc;
        for (auto& [rank, amount] : randomMissionsCompleted)
        {
            doc.append(B_KVP(rank, amount));
        }
        document.append(B_KVP("randomMissionsCompleted", doc));
    }

    {
        B_DOC doc;
        for (auto& [rank, amount] : randomMissionsAborted)
        {
            doc.append(B_KVP(rank, amount));
        }
        document.append(B_KVP("randomMissionsAborted", doc));
    }

    {
        B_DOC doc;
        for (auto& [rank, amount] : randomMissionsFailed)
        {
            doc.append(B_KVP(rank, amount));
        }
        document.append(B_KVP("randomMissionsFailed", doc));
    }

    {
        B_ARR arr;
        for (auto& [spaceObj, visitFlag] : visits)
        {
            arr.append(B_MARR(spaceObj, visitFlag));
        }
        document.append(B_KVP("visits", arr));
    }

    {
        B_ARR arr;
        for (auto sysId : systemsVisited)
        {
            arr.append(sysId);
        }
        document.append(B_KVP("systemsVisited", arr));
    }

    {
        B_ARR arr;
        for (auto baseId : basesVisited)
        {
            arr.append(baseId);
        }
        document.append(B_KVP("basesVisited", arr));
    }

    {
        B_ARR arr;
        for (const auto& [id, baseId, interactionCount, missionStatus] : npcVisits)
        {
            arr.append(B_MDOC(B_KVP("id", id), B_KVP("baseId", baseId), B_KVP("interactionCount", interactionCount), B_KVP("missionStatus", missionStatus)));
        }

        document.append(B_KVP("npcVisits", arr));
    }

    {
        auto arr = B_ARR{};
        for (auto jumpHole : jumpHolesVisited)
        {
            arr.append(jumpHole);
        }

        document.append(B_KVP("jumpHolesVisited", arr));
    }

    {
        auto arr = B_ARR{};
        for (auto& [rumorIds, priority] : rumorsReceived)
        {
            arr.append(B_MDOC(B_KVP("rumorIds", rumorIds), B_KVP("priority", priority)));
        }

        document.append(B_KVP("rumorsReceived", arr));
    }

    {
        B_DOC doc;
        for (auto& [key, wpnGrp] : weaponGroups)
        {
            B_ARR arr;
            for (auto& wpnGroup : wpnGrp)
            {
                arr.append(wpnGroup);
            }
            doc.append(B_KVP(key, arr));
        }
        document.append(B_KVP("weaponGroups", doc));
    }

    if (lastRenameTimestamp.has_value())
    {
        document.append(B_KVP("lastRenameTimestamp", bsoncxx::types::b_date{ static_cast<std::chrono::milliseconds>(lastRenameTimestamp.value()) }));
    }

    if (characterTransferCode.has_value())
    {
        document.append(B_KVP("characterTransferCode", characterTransferCode.value()));
    }

    {
        B_ARR arr;
        for (std::string msg : presetMsgs)
        {
            arr.append(msg);
        }
        document.append(B_KVP("presetMsgs", arr));
    }
}
