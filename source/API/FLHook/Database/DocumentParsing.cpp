#include "PCH.hpp"

#include "API/FLHook/AccountManager.hpp"

using bsoncxx::builder::basic::kvp;
using bsoncxx::builder::basic::make_array;
using bsoncxx::builder::basic::make_document;

Account::Account(const bsoncxx::document::view view)
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

void Account::ToBson(bsoncxx::builder::basic::document& document) const
{
    document.append(kvp("_id", _id));
    document.append(kvp("banned", banned));
    document.append(kvp("cash", cash));

    {
        bsoncxx::builder::basic::array arr;
        for (auto& character : characters)
        {
            arr.append(character);
        }
        document.append(kvp("characters", arr));
    }

    if (scheduledUnbanDate.has_value())
    {
        document.append(kvp("scheduledUnbanDate", scheduledUnbanDate.value()));
    }

    if (gameRoles.has_value())
    {
        bsoncxx::builder::basic::array arr;
        for (auto& role : gameRoles.value())
        {
            arr.append(role);
        }
        document.append(kvp("gameRoles", arr));
    }

    if (webRoles.has_value())
    {
        bsoncxx::builder::basic::array arr;
        for (auto& role : webRoles.value())
        {
            arr.append(role);
        }
        document.append(kvp("webRoles", arr));
    }

    if (hashedToken.has_value())
    {
        document.append(kvp("hashedToken", hashedToken.value()));
    }

    if (username.has_value())
    {
        document.append(kvp("username", username.value()));
    }

    if (passwordHash.has_value())
    {
        document.append(kvp("passwordHash", passwordHash.value()));
    }

    if (salt.has_value())
    {
        bsoncxx::types::b_binary data{ bsoncxx::binary_sub_type::k_binary, salt->size(), salt->data() };
        document.append(kvp("salt", data));
    }
}

Character::Character(bsoncxx::document::view view)
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

void Character::ToBson(bsoncxx::builder::basic::document& document)
{
    HandleDeadCharacter();

    if (_id.has_value())
    {
        document.append(kvp("_id", _id.value()));
    }

    document.append(kvp("accountId", accountId));
    document.append(kvp("characterName", characterName));
    document.append(kvp("money", money));
    document.append(kvp("rank", rank));
    document.append(kvp("affiliation", affiliation));

    if (repGroup.has_value())
    {
        document.append(kvp("repGroup", repGroup.value()));
    }

    document.append(kvp("pos", make_array(pos.x, pos.y, pos.z)));
    document.append(kvp("rot", make_array(rot.x, rot.y, rot.z)));
    document.append(kvp("voice", voice));
    document.append(kvp("interfaceState", interfaceState));
    document.append(kvp("hullStatus", hullStatus));
    document.append(kvp("baseHullStatus", baseHullStatus));
    document.append(kvp("canDock", canDock));
    document.append(kvp("canTradeLane", canTradeLane));

    if (tlExceptions.has_value())
    {
        auto arr = bsoncxx::builder::basic::array{};
        for (auto& [startRingId, nextRingId] : tlExceptions.value())
        {
            arr.append(make_document(kvp("startRingId", startRingId), kvp("nextRingId", nextRingId)));
        }
        document.append(kvp("tlExceptions", arr));
    }

    if (dockExceptions.has_value())
    {
        auto arr = bsoncxx::builder::basic::array{};
        for (auto& base : dockExceptions.value())
        {
            arr.append(base);
        }
        document.append(kvp("dockExceptions", arr));
    }
    document.append(kvp("lastDockedBase", lastDockedBase));
    document.append(kvp("currentBase", currentBase));
    document.append(kvp("currentRoom", currentRoom));
    document.append(kvp("killCount", killCount));
    document.append(kvp("missionFailureCount", missionFailureCount));
    document.append(kvp("missionSuccessCount", missionSuccessCount));
    document.append(kvp("shipHash", shipHash));
    document.append(kvp("system", system));
    document.append(kvp("totalTimePlayed", totalTimePlayed));
    document.append(kvp("totalCashEarned", totalCashEarned));
    // clang-format off
    document.append(kvp("baseCostume", make_document(
        kvp("head", static_cast<int>(baseCostume.head)),
        kvp("body", static_cast<int>(baseCostume.body)),
        kvp("leftHand", static_cast<int>(baseCostume.leftHand)),
        kvp("rightHand", static_cast<int>(baseCostume.rightHand)),
        kvp("accessories", static_cast<int>(baseCostume.accessories)),
        kvp("accessory", make_array(
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

    document.append(kvp("commCostume", make_document(
        kvp("head", static_cast<int>(commCostume.head)),
        kvp("body", static_cast<int>(commCostume.body)),
        kvp("leftHand", static_cast<int>(commCostume.leftHand)),
        kvp("rightHand", static_cast<int>(commCostume.rightHand)),
        kvp("accessories", static_cast<int>(commCostume.accessories)),
        kvp("accessory", make_array(
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
        bsoncxx::builder::basic::array arr;
        for (const auto& [archId, amount, health, isMissionCargo] : cargo)
        {
            arr.append(make_document(kvp("archId", archId), kvp("amount", amount), kvp("health", health), kvp("isMissionCargo", isMissionCargo)));
        }
        document.append(kvp("cargo", arr));
    }

    {
        bsoncxx::builder::basic::array arr;
        for (const auto& [archId, amount, health, isMissionCargo] : baseCargo)
        {
            arr.append(make_document(kvp("archId", archId), kvp("amount", amount), kvp("health", health), kvp("isMissionCargo", isMissionCargo)));
        }
        document.append(kvp("baseCargo", arr));
    }

    {
        bsoncxx::builder::basic::array arr;
        for (const auto& [archId, hardPoint, health, amount, mounted] : equipment)
        {
            arr.append(
                make_document(kvp("archId", archId), kvp("hardPoint", hardPoint), kvp("health", health), kvp("mounted", mounted), kvp("amount", amount)));
        }
        document.append(kvp("equipment", arr));
    }

    {
        bsoncxx::builder::basic::array arr;
        for (const auto& [archId, hardPoint, health, amount, mounted] : baseEquipment)
        {
            arr.append(
                make_document(kvp("archId", archId), kvp("hardPoint", hardPoint), kvp("health", health), kvp("mounted", mounted), kvp("amount", amount)));
        }
        document.append(kvp("baseEquipment", arr));
    }

    {
        bsoncxx::builder::basic::document doc;
        for (auto& [id, health] : collisionGroups)
        {
            doc.append(kvp(id, health));
        }
        document.append(kvp("collisionGroups", doc));
    }

    {
        bsoncxx::builder::basic::document doc;
        for (auto& [id, health] : baseCollisionGroups)
        {
            doc.append(kvp(id, health));
        }
        document.append(kvp("baseCollisionGroups", doc));
    }

    {
        bsoncxx::builder::basic::document doc;
        for (auto& [faction, rep] : reputation)
        {
            doc.append(kvp(faction, rep));
        }
        document.append(kvp("reputation", doc));
    }

    {
        bsoncxx::builder::basic::document doc;
        for (auto& [shipArch, amount] : shipTypesKilled)
        {
            doc.append(kvp(shipArch, amount));
        }
        document.append(kvp("shipTypesKilled", doc));
    }

    {
        bsoncxx::builder::basic::document doc;
        for (auto& [rank, amount] : randomMissionsCompleted)
        {
            doc.append(kvp(rank, amount));
        }
        document.append(kvp("randomMissionsCompleted", doc));
    }

    {
        bsoncxx::builder::basic::document doc;
        for (auto& [rank, amount] : randomMissionsAborted)
        {
            doc.append(kvp(rank, amount));
        }
        document.append(kvp("randomMissionsAborted", doc));
    }

    {
        bsoncxx::builder::basic::document doc;
        for (auto& [rank, amount] : randomMissionsFailed)
        {
            doc.append(kvp(rank, amount));
        }
        document.append(kvp("randomMissionsFailed", doc));
    }

    {
        bsoncxx::builder::basic::array arr;
        for (auto& [spaceObj, visitFlag] : visits)
        {
            arr.append(make_array(spaceObj, visitFlag));
        }
        document.append(kvp("visits", arr));
    }

    {
        bsoncxx::builder::basic::array arr;
        for (auto sysId : systemsVisited)
        {
            arr.append(sysId);
        }
        document.append(kvp("systemsVisited", arr));
    }

    {
        bsoncxx::builder::basic::array arr;
        for (auto baseId : basesVisited)
        {
            arr.append(baseId);
        }
        document.append(kvp("basesVisited", arr));
    }

    {
        bsoncxx::builder::basic::array arr;
        for (const auto& [id, baseId, interactionCount, missionStatus] : npcVisits)
        {
            arr.append(make_document(kvp("id", id), kvp("baseId", baseId), kvp("interactionCount", interactionCount), kvp("missionStatus", missionStatus)));
        }

        document.append(kvp("npcVisits", arr));
    }

    {
        auto arr = bsoncxx::builder::basic::array{};
        for (auto jumpHole : jumpHolesVisited)
        {
            arr.append(jumpHole);
        }

        document.append(kvp("jumpHolesVisited", arr));
    }

    {
        auto arr = bsoncxx::builder::basic::array{};
        for (auto& [rumorIds, priority] : rumorsReceived)
        {
            arr.append(make_document(kvp("rumorIds", rumorIds), kvp("priority", priority)));
        }

        document.append(kvp("rumorsReceived", arr));
    }

    {
        bsoncxx::builder::basic::document doc;
        for (auto& [key, wpnGrp] : weaponGroups)
        {
            bsoncxx::builder::basic::array arr;
            for (auto& wpnGroup : wpnGrp)
            {
                arr.append(wpnGroup);
            }
            doc.append(kvp(key, arr));
        }
        document.append(kvp("weaponGroups", doc));
    }

    if (lastRenameTimestamp.has_value())
    {
        document.append(kvp("lastRenameTimestamp", bsoncxx::types::b_date{ static_cast<std::chrono::milliseconds>(lastRenameTimestamp.value()) }));
    }

    if (characterTransferCode.has_value())
    {
        document.append(kvp("characterTransferCode", characterTransferCode.value()));
    }

    {
        bsoncxx::builder::basic::array arr;
        for (std::string msg : presetMsgs)
        {
            arr.append(msg);
        }
        document.append(kvp("presetMsgs", arr));
    }
}
