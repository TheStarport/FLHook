#include "PCH.hpp"

#include "API/FLHook/AccountManager.hpp"

DbAccount::DbAccount(const B_VIEW view)
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
            case Hash("cash"):
                {
                    cash = element.get_int64();
                    break;
                }
            case Hash("scheduledUnbanDate"):
                {
                    if (element.type() == bsoncxx::type::k_int64)
                    {
                        scheduledUnbanDate = element.get_int64();
                    }
                    break;
                }
            case Hash("gameRoles"):
                {
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    gameRoles = std::vector<std::string>{};
                    for (auto id : element.get_array().value)
                    {
                        gameRoles->emplace_back(id.get_string().value);
                    }
                    break;
                }
            case Hash("webRoles"):
                {
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    webRoles = std::vector<std::string>{};
                    for (auto id : element.get_array().value)
                    {
                        webRoles->emplace_back(id.get_string().value);
                    }
                    break;
                }
            case Hash("hashedToken"):
                {
                    if (element.type() != bsoncxx::type::k_string)
                    {
                        break;
                    }

                    hashedToken = element.get_string();
                    break;
                }
            case Hash("username"):
                {
                    if (element.type() != bsoncxx::type::k_string)
                    {
                        break;
                    }

                    username = element.get_string();
                    break;
                }
            case Hash("passwordHash"):
                {
                    if (element.type() != bsoncxx::type::k_string)
                    {
                        break;
                    }

                    passwordHash = element.get_string();
                    break;
                }
            case Hash("salt"):
                {
                    if (element.type() != bsoncxx::type::k_binary)
                    {
                        break;
                    }

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

void DbAccount::ToBson(B_DOC& document) const
{
    document.append(B_KVP("_id", _id));
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
                    if (element.type() != bsoncxx::type::k_int32)
                    {
                        break;
                    }

                    money = element.get_int32().value;
                    break;
                }
            case Hash("rank"):
                {
                    if (element.type() != bsoncxx::type::k_int32)
                    {
                        break;
                    }

                    rank = element.get_int32().value;
                    break;
                }
            case Hash("repGroup"):
                {
                    if (element.type() != bsoncxx::type::k_string)
                    {
                        break;
                    }

                    repGroup = element.get_string().value;
                    break;
                }
            case Hash("pos"):
                {
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    int counter = 0;
                    for (auto& var : element.get_array().value)
                    {
                        pos[counter++] = static_cast<float>(var.get_double().value);
                    }
                    break;
                }
            case Hash("rot"):
                {
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    int counter = 0;
                    for (auto& var : element.get_array().value)
                    {
                        rot[counter++] = static_cast<float>(var.get_double().value);
                    }
                    break;
                }
            case Hash("voice"):
                {
                    if (element.type() != bsoncxx::type::k_string)
                    {
                        break;
                    }

                    voice = element.get_string().value;
                    break;
                }
            case Hash("interfaceState"):
                {
                    if (element.type() != bsoncxx::type::k_int32)
                    {
                        break;
                    }

                    interfaceState = element.get_int32().value;
                    break;
                }
            case Hash("hullStatus"):
                {
                    if (element.type() != bsoncxx::type::k_double)
                    {
                        break;
                    }

                    hullStatus = static_cast<float>(element.get_double().value);
                    break;
                }
            case Hash("baseHullStatus"):
                {
                    if (element.type() != bsoncxx::type::k_double)
                    {
                        break;
                    }

                    baseHullStatus = static_cast<float>(element.get_double().value);
                    break;
                }
            case Hash("canDock"):
                {
                    if (element.type() != bsoncxx::type::k_bool)
                    {
                        break;
                    }

                    canDock = element.get_bool().value;
                    break;
                }
            case Hash("canTradeLane"):
                {
                    if (element.type() != bsoncxx::type::k_bool)
                    {
                        break;
                    }

                    canTradeLane = element.get_bool().value;
                    break;
                }
            case Hash("tlExceptions"):
                {
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    tlExceptions = std::vector<TradeLaneException>{};
                    for (auto& tl : element.get_array().value)
                    {
                        auto doc = tl.get_document().value;
                        TradeLaneException exception{};

                        for (auto& el : doc)
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
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    dockExceptions = std::vector<int>{};
                    for (auto& baseId : element.get_array().value)
                    {
                        dockExceptions->emplace_back(baseId.get_int32().value);
                    }
                    break;
                }
            case Hash("lastDockedBase"):
                {
                    if (element.type() != bsoncxx::type::k_int32)
                    {
                        break;
                    }

                    lastDockedBase = element.get_int32().value;
                    break;
                }
            case Hash("currentBase"):
                {
                    if (element.type() != bsoncxx::type::k_int32)
                    {
                        break;
                    }

                    currentBase = element.get_int32().value;
                    break;
                }
            case Hash("currentRoom"):
                {
                    if (element.type() != bsoncxx::type::k_int32)
                    {
                        break;
                    }

                    currentRoom = element.get_int32().value;
                    break;
                }
            case Hash("killCount"):
                {
                    if (element.type() != bsoncxx::type::k_int32)
                    {
                        break;
                    }

                    killCount = element.get_int32().value;
                    break;
                }
            case Hash("missionFailureCount"):
                {
                    if (element.type() != bsoncxx::type::k_int32)
                    {
                        break;
                    }

                    missionFailureCount = element.get_int32().value;
                    break;
                }
            case Hash("missionSuccessCount"):
                {
                    if (element.type() != bsoncxx::type::k_int32)
                    {
                        break;
                    }

                    missionSuccessCount = element.get_int32().value;
                    break;
                }
            case Hash("shipHash"):
                {
                    if (element.type() != bsoncxx::type::k_int32)
                    {
                        break;
                    }

                    shipHash = element.get_int32().value;
                    break;
                }
            case Hash("system"):
                {
                    if (element.type() != bsoncxx::type::k_int32)
                    {
                        break;
                    }

                    system = element.get_int32().value;
                    break;
                }
            case Hash("totalTimePlayed"):
                {
                    if (element.type() != bsoncxx::type::k_double)
                    {
                        break;
                    }

                    totalTimePlayed = static_cast<float>(element.get_double().value);
                    break;
                }
            case Hash("baseCostume"):
                {
                    if (element.type() != bsoncxx::type::k_document)
                    {
                        break;
                    }

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
                        else if (key == "accessories")
                        {
                            int counter = 0;
                            for (auto& acc : el.get_array().value)
                            {
                                baseCostume.accessory[counter++] = static_cast<uint>(acc.get_int32().value);
                            }
                            baseCostume.accessories = counter;
                        }
                    }
                    break;
                }
            case Hash("commCostume"):
                {
                    if (element.type() != bsoncxx::type::k_document)
                    {
                        break;
                    }

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
                        else if (key == "accessories")
                        {
                            int counter = 0;
                            for (auto& acc : el.get_array().value)
                            {
                                commCostume.accessory[counter++] = static_cast<uint>(acc.get_int32().value);
                            }
                            commCostume.accessories = counter;
                        }
                    }
                    break;
                }
            case Hash("cargo"):
                {
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    cargo = std::vector<FLCargo>{};
                    for (auto& item : element.get_array().value)
                    {
                        auto doc = item.get_document().value;
                        FLCargo cargoItem{};

                        for (auto& el : doc)
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
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    baseCargo = std::vector<FLCargo>{};
                    for (auto& item : element.get_array().value)
                    {
                        auto doc = item.get_document().value;
                        FLCargo cargoItem{};

                        for (auto& el : doc)
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
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    equipment = std::vector<Equipment>{};
                    for (auto& item : element.get_array().value)
                    {
                        auto doc = item.get_document().value;
                        Equipment eq;

                        for (auto& el : doc)
                        {
                            if (std::string_view key = el.key(); key == "archId")
                            {
                                eq.archId = el.get_int32().value;
                            }
                            else if (key == "hp")
                            {
                                eq.hardPoint = el.get_string().value;
                            }
                            else if (key == "health")
                            {
                                eq.health = static_cast<float>(el.get_double().value);
                            }
                        }

                        equipment.emplace_back(eq);
                    }
                    break;
                }
            case Hash("baseEquipment"):
                {
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    baseEquipment = std::vector<Equipment>{};
                    for (auto& item : element.get_array().value)
                    {
                        auto doc = item.get_document().value;
                        Equipment eq;

                        for (auto& el : doc)
                        {
                            if (std::string_view key = el.key(); key == "archId")
                            {
                                eq.archId = el.get_int32().value;
                            }
                            else if (key == "hp")
                            {
                                eq.hardPoint = el.get_string().value;
                            }
                            else if (key == "health")
                            {
                                eq.health = static_cast<float>(el.get_double().value);
                            }
                        }

                        baseEquipment.emplace_back(eq);
                    }
                    break;
                }
            case Hash("collisionGroups"):
                {
                    if (element.type() != bsoncxx::type::k_document)
                    {
                        break;
                    }

                    for (auto& el : element.get_document().value)
                    {
                        collisionGroups[std::string(el.key())] = static_cast<float>(el.get_double().value);
                    }
                    break;
                }
            case Hash("baseCollisionGroups"):
                {
                    if (element.type() != bsoncxx::type::k_document)
                    {
                        break;
                    }

                    for (auto& el : element.get_document().value)
                    {
                        baseCollisionGroups[std::string(el.key())] = static_cast<float>(el.get_double().value);
                    }
                    break;
                }
            case Hash("reputation"):
                {
                    if (element.type() != bsoncxx::type::k_document)
                    {
                        break;
                    }

                    for (auto& el : element.get_document().value)
                    {
                        reputation[std::string(el.key())] = static_cast<float>(el.get_double().value);
                    }
                    break;
                }
            case Hash("shipTypesKilled"):
                {
                    if (element.type() != bsoncxx::type::k_document)
                    {
                        break;
                    }

                    for (auto& el : element.get_document().value)
                    {
                        shipTypesKilled[std::string(el.key())] = el.get_int32().value;
                    }
                    break;
                }
            case Hash("randomMissionsCompleted"):
                {
                    if (element.type() != bsoncxx::type::k_document)
                    {
                        break;
                    }

                    for (auto& el : element.get_document().value)
                    {
                        randomMissionsCompleted[std::string(el.key())] = el.get_int32().value;
                    }
                    break;
                }
            case Hash("randomMissionsAborted"):
                {
                    if (element.type() != bsoncxx::type::k_document)
                    {
                        break;
                    }

                    for (auto& el : element.get_document().value)
                    {
                        randomMissionsAborted[std::string(el.key())] = el.get_int32().value;
                    }
                    break;
                }
            case Hash("randomMissionsFailed"):
                {
                    if (element.type() != bsoncxx::type::k_document)
                    {
                        break;
                    }

                    for (auto& el : element.get_document().value)
                    {
                        randomMissionsFailed[std::string(el.key())] = el.get_int32().value;
                    }
                    break;
                }
            case Hash("visits"):
                {
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    for (auto& el : element.get_array().value)
                    {
                        auto array = el.get_array().value;
                        visits.emplace_back(std::array{ array[0].get_int32().value, array[1].get_int32().value });
                    }
                    break;
                }
            case Hash("systemsVisited"):
                {
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    systemsVisited = std::vector<int>{};
                    for (auto& baseId : element.get_array().value)
                    {
                        systemsVisited.emplace_back(baseId.get_int32().value);
                    }
                    break;
                }
            case Hash("basesVisited"):
                {
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    basesVisited = std::vector<int>{};
                    for (auto& baseId : element.get_array().value)
                    {
                        basesVisited.emplace_back(baseId.get_int32().value);
                    }
                    break;
                }
            case Hash("npcVisits"):
                {
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

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
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    jumpHolesVisited = std::vector<int>{};
                    for (auto& jumpHoleId : element.get_array().value)
                    {
                        jumpHolesVisited.emplace_back(jumpHoleId.get_int32().value);
                    }
                    break;
                }
            case Hash("rumorsReceived"):
                {
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    rumorsReceived = std::vector<RumorData>{};
                    for (auto& rumorData : element.get_array().value)
                    {
                        auto doc = rumorData.get_document().value;
                        RumorData rumor{};

                        for (auto& el : doc)
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
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    for (auto& el : element.get_document().value)
                    {
                        auto group = weaponGroups[std::string(el.key())] = {};
                        for (auto& weapon : el.get_array().value)
                        {
                            group.emplace_back(weapon.get_string().value);
                        }
                    }
                    break;
                }
            case Hash("lastRenameTimestamp"):
                {
                    if (element.type() != bsoncxx::type::k_int64)
                    {
                        break;
                    }

                    lastRenameTimestamp = element.get_date().to_int64();
                    break;
                }
            case Hash("characterTransferCode"):
                {
                    if (element.type() != bsoncxx::type::k_string)
                    {
                        break;
                    }

                    characterTransferCode = element.get_string().value;
                    break;
                }
            case Hash("presetMsgs"):
                {
                    if (element.type() != bsoncxx::type::k_array)
                    {
                        break;
                    }

                    presetMsgs = std::array<std::string, 10>{};
                    uint i = 0;
                    for (auto& message : element.get_array().value)
                    {
                        presetMsgs.at(i) = message.get_string().value;
                        i++;
                    }
                    break;
                }
            default: break;
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
    // clang-format off
    
    B_ARR baseAccessories;
    for (int i = 0; i != baseCostume.accessories; i++)
    {
        baseAccessories.append(static_cast<int>(baseCostume.accessory[i]));
    }
    document.append(B_KVP("baseCostume", B_MDOC(
        B_KVP("head", static_cast<int>(baseCostume.head)),
        B_KVP("body", static_cast<int>(baseCostume.body)),
        B_KVP("leftHand", static_cast<int>(baseCostume.leftHand)),
        B_KVP("rightHand", static_cast<int>(baseCostume.rightHand)),
        B_KVP("accessories", baseAccessories))
        ));

    B_ARR commAccessories;
    for (int i = 0; i != commCostume.accessories; i++)
    {
        commAccessories.append(static_cast<int>(commCostume.accessory[i]));
    }
    document.append(B_KVP("commCostume", B_MDOC(
        B_KVP("head", static_cast<int>(commCostume.head)),
        B_KVP("body", static_cast<int>(commCostume.body)),
        B_KVP("leftHand", static_cast<int>(commCostume.leftHand)),
        B_KVP("rightHand", static_cast<int>(commCostume.rightHand)),
        B_KVP("accessories", commAccessories))));
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
        for (const auto& [archId, hardPoint, health] : equipment)
        {
            arr.append(B_MDOC(B_KVP("archId", archId), B_KVP("hp", hardPoint), B_KVP("health", health)));
        }
        document.append(B_KVP("equipment", arr));
    }

    {
        B_ARR arr;
        for (const auto& [archId, hardPoint, health] : baseEquipment)
        {
            arr.append(B_MDOC(B_KVP("archId", archId), B_KVP("hp", hardPoint), B_KVP("health", health)));
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
