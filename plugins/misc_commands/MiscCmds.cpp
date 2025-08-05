#include "PCH.hpp"

#include "MiscCmds.hpp"

#include "API/Utils/Random.hpp"
#include "API/FLHook/InfocardManager.hpp"

namespace Plugins
{

    concurrencpp::result<void> MiscCmdsPlugin::UserCmdBountyScan(ClientId client)
    {
        auto ship = client.GetShip().Handle();
        auto target = ship.GetTarget().Handle();
        auto targetClient = target.GetPlayer().Handle();

        const Archetype::Tractor* itemPtr = nullptr;
        for (auto& equip : targetClient.GetEquipCargo().Handle()->equip)
        {
            if (!equip.mounted)
            {
                continue;
            }
       
            itemPtr = dynamic_cast<Archetype::Tractor*>(Archetype::GetEquipment(equip.archId.GetValue()));
            if (itemPtr)
            {
                break;
            }
        }

        std::wstring targetId = L"No mounted ID!";
        if (itemPtr != nullptr)
        {
            const GoodInfo* idInfo = GoodList_get()->find_by_archetype(itemPtr->get_id());
            if (idInfo == nullptr) // maybe a funny developer put one in the game as equipment but not a good?
            {
                targetId = L"Unknown " + std::to_wstring(itemPtr->archId.GetValue());
            }
            else
            {
                targetId = FLHook::GetInfocardManager()->GetInfoName(idInfo->idsName);
            }
        }

        // ship
        const GoodInfo* shipInfo = GoodList_get()->find_by_ship_arch(target.GetArchetype().Handle()->archId.GetValue());
        std::wstring targetShipArchName = L"Unknown " + std::to_wstring(target.GetArchetype().Handle()->archId.GetValue());
        if (shipInfo != nullptr)
        {
            targetShipArchName = FLHook::GetInfocardManager()->GetInfoName(shipInfo->idsName);
        }

        // output
        client.Message(std::format(L"{} - {}", targetClient.GetCharacterId().Handle(), targetShipArchName));
        client.Message(std::format(L"{} - {}", targetId, target.GetReputation().Handle().GetAffiliation().Handle().GetName().Handle()));
        client.Message(std::format(L"System - {}", target.GetSystem().Handle().GetName().Handle()));
        client.Message(std::format(L"Finished bounty scan at {}", TimeUtils::CurrentDateTimeString()));

        co_return;
    }

    int GetMembersInSpace(GroupId group)
    {
        if (!group)
        {
            return 0;
        }

        uint membersInSpace = 0;

        for (auto& member : group.GetGroupMembers().Handle())
        {
            if (member.InSpace())
            {
                membersInSpace++;
            }
        }

        return membersInSpace;
    }

    concurrencpp::result<void> MiscCmdsPlugin::UserCmdGroupSize(ClientId client, std::optional<GroupId> groupId)
    {
        auto group = client.GetGroup().Unwrap();
        GroupId targetGroupId;

        if (groupId.has_value())
        {
            targetGroupId = groupId.value();
        }
        else
        {
            auto ship = client.GetShip().Unwrap();
            if (ship)
            {
                auto target = ship.GetTarget().Unwrap();
                if (target)
                {
                    auto targetPlayer = target.GetPlayer().Unwrap();
                    if (targetPlayer)
                    {
                        auto group = targetPlayer.GetGroup().Unwrap();
                        if (group)
                        {
                            targetGroupId = group;
                        }
                    }
                }
            }
        }
        auto targetGroup = CPlayerGroup::FromGroupID(targetGroupId.GetValue());

        if (!targetGroup || !targetGroup->memberCount)
        {
            if (group)
            {
                client.Message(std::format(L"Your group size: {} ({} in space)", group.GetGroupSize().Handle(), GetMembersInSpace(group)));
            }
            else
            {
                client.MessageErr(L"No parameter provided, target not in a group, and you're not in a group");
            }
            co_return;
        }

        client.Message(std::format(L"Target group size: {} ({} in space)", targetGroup->GetMemberCount(), GetMembersInSpace(targetGroupId)));
        if (group && group.GetValue() != targetGroupId.GetValue())
        {
            client.Message(std::format(L"Your group size: {} ({} in space)", group.GetGroupSize().Handle(), GetMembersInSpace(group)));
        }
        
        co_return;
    }

    MiscCmdsPlugin::MiscCmdsPlugin(const PluginInfo& info) : Plugin(info) {}
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"Miscellaneous Commands",
	    .shortName = L"misc_cmds",
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(MiscCmdsPlugin);
