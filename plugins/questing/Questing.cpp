#include "PCH.hpp"

#include "Questing.hpp"

#include "API/FLHook/ClientList.hpp"
#include "API/FLHook/ResourceManager.hpp"

using namespace std::string_view_literals;

namespace Plugins
{
    QuestingPlugin::QuestingPlugin(const PluginInfo& info) : Plugin(info) {}

    void QuestingPlugin::CurrentStageTimer()
    {
        std::vector<uint> questsToComplete;
        for (auto& [group, instance] : activeQuests)
        {
            // Quest marked as over
            if (instance->timeUntilCleanup != UINT_MAX || !instance->questStarted)
            {
                if (instance->timeUntilCleanup <= TimeUtils::UnixTime<std::chrono::seconds>())
                {
                    questsToComplete.emplace_back(group);
                }

                continue;
            }

            const auto& currentStage = instance->stages[instance->currentStage];

            sol::protected_function_result result = currentStage.shouldChangeStage();
            if (!result.valid())
            {
                instance->timeUntilCleanup = 1;
                sol::error err = result;
                ERROR(L"Unable to run should_change_stage {0}    {1}",
                      { L"file", StringUtils::stows(instance->info->luaFile) },
                      { L"error", StringUtils::stows(err.what()) });
                continue;
            }

            sol::optional<uint> newStageValue = result;
            if (!newStageValue.has_value())
            {
                continue;
            }

            auto newStage = instance->stages.find(*newStageValue);
            if (newStage == instance->stages.end())
            {
                instance->timeUntilCleanup = 1;
                ERROR(L"Unable to find {0} in quest", { L"stage", std::to_wstring(*newStageValue) });
                continue;
            }

            instance->currentStage = newStageValue.value();
            if (newStage->second.onStageBegin.has_value())
            {
                if (result = newStage->second.onStageBegin.value()(); !result.valid())
                {
                    instance->timeUntilCleanup = 1;
                    sol::error err = result;
                    ERROR(L"Unable to run on_stage_begin {0}    {1}",
                          { L"file", StringUtils::stows(instance->info->luaFile) },
                          { L"error", StringUtils::stows(err.what()) });
                    continue;
                }
            }

            if (!newStage->second.missionFail.has_value())
            {
                continue;
            }

            pub::Audio::Tryptich music;
            music.overrideMusic = CreateID(newStage->second.missionFail.value() ? "music_failure" : "music_victory");
            auto gId = GroupId(group);
            gId.ForEachGroupMember(
                [&music](const ClientId client)
                {
                    client.PlayMusic(music);
                    return std::nullopt;
                },
                false);

            instance->timeUntilCleanup = TimeUtils::UnixTime<std::chrono::seconds>() + instance->info->cleanUpTime.value_or(300);
        }

        for (auto group : questsToComplete)
        {
            activeQuests.erase(group);
        }
    }

    /// Load the configuration
    bool QuestingPlugin::OnLoadSettings()
    {
        constexpr auto path = "../DATA/SCRIPTS/LUA/QUESTS"sv;
        if (!std::filesystem::exists(path))
        {
            INFO(L"'../DATA/SCRIPTS/LUA/QUESTS' not found, unloading questing module");
            return false;
        }

        for (const auto& dir : std::filesystem::directory_iterator(path))
        {
            if (!dir.is_regular_file() || dir.path().extension() != ".lua")
            {
                continue;
            }

            sol::state lua;
            SetupLuaState(&lua);

            QuestInfo info;
            lua["quest_info"] = &info;
            if (auto loadRes = lua.do_file(dir.path().string()); !loadRes.valid())
            {
                sol::error err = loadRes;
                ERROR(L"Unable to load lua {0}    {1}", { L"file", dir.path().wstring() }, { L"error", StringUtils::stows(err.what()) });
                continue;
            }

            if (info.questName.empty())
            {
                ERROR(L"Quest name was not specified in lua {0}", { L"file", dir.path().wstring() });
                continue;
            }

            if (const sol::optional<sol::table> stages = lua["stages"]; !stages || stages->empty() || !stages->operator[](0).valid())
            {
                ERROR(L"{0} has no stages or no starting stage", { L"file", dir.path().wstring() });
                continue;
            }
            else
            {
                for (const auto& stage : *stages)
                {
                    if (stage.first.get_type() != sol::type::number)
                    {
                        ERROR(L"{0} has a stage that does not have an integer key", { L"file", dir.path().wstring() });
                        continue;
                    }

                    if (stage.first.as<int>() < 0)
                    {
                        ERROR(L"{0} has a stage that has a negative integer key", { L"file", dir.path().wstring() });
                        continue;
                    }

                    if (!stage.second.is<QuestStage>())
                    {
                        ERROR(L"{0} has a stage value that is not of type QuestStage", { L"file", dir.path().wstring() });
                        continue;
                    }
                }
            }

            info.luaFile = dir.path().string();
            quests.emplace_back(info);
        }

        return !quests.empty();
    }

    void QuestingPlugin::OnServerStartupAfter()
    {
        // Every second check in with the quests to see if any need to progress
        AddTimer([this] { CurrentStageTimer(); }, 1000);
    }
    void QuestingPlugin::OnBaseEnterAfter(BaseId base, const ClientId client)
    {
        auto currentGroup = client.GetGroup();
        if (currentGroup.HasError())
        {
            return;
        }

        for (auto& [group, instance] : activeQuests)
        {
            if (instance->timeUntilCleanup != UINT_MAX && group == currentGroup.Unwrap().GetValue() &&
                instance->questStarter == client.GetCharacterName().Handle())
            {
                instance->timeUntilCleanup = 1; // Clean up now!
                return;
            }
        }
    }

    concurrencpp::result<void> QuestingPlugin::UserCmdStartQuest(const ClientId client, const StrToEnd questName)
    {
        if (questName.end.empty())
        {
            client.Message(L"No quest name provided!");

            co_return;
        }

        auto quest = std::ranges::find_if(quests, [questName](const auto& q) { return q.questName == questName.end; });
        if (quest == quests.end())
        {
            client.Message(std::format(L"Quest '{}' not found", questName.end));
            co_return;
        }

        GroupId groupId;
        if (auto groupAct = client.GetGroup(); groupAct.HasError())
        {
            // ReSharper disable once CppDFAMemoryLeak
            auto* newGroup = new CPlayerGroup();
            newGroup->AddMember(client.GetValue());
            groupId = GroupId(newGroup->GetID());
        }
        else
        {
            groupId = groupAct.Value();
        }

        const auto& instance = activeQuests[groupId.GetValue()] = std::make_unique<QuestInstance>(&*quest);
        SetupLuaState(&instance->lua);
        instance->lua["quest_info"] = instance->info;
        instance->lua["group"] = groupId;

        if (const auto result = instance->lua.do_file(quest->luaFile); !result.valid())
        {
            const sol::error err = result;
            ERROR(L"Unable to load lua {0}    {1}", { L"file", StringUtils::stows(quest->luaFile) }, { L"error", StringUtils::stows(err.what()) });

            client.Message(std::format(L"DEV ERROR: unable to start quest '{}', please report this to your server administrator.", quest->questName));
            instance->timeUntilCleanup = 0; // Cleanup immediately
            co_return;
        }

        instance->questStarter = std::wstring(client.GetCharacterName().Handle());

        for (auto [key, value] : instance->lua["stages"].get<sol::table>())
        {
            uint stageKey = key.as<uint>();
            const auto stage = value.as<QuestStage>();

            instance->stages[stageKey] = stage;
        }

        client.Message(std::format(L"Quest '{}' started! Gather your squad mates and /quest confirm when you are ready to begin.", questName.end));
        co_return;
    }

    concurrencpp::result<void> QuestingPlugin::UserCmdListQuests(const ClientId client) const
    {
        for (const auto& quest : quests)
        {
            // TODO: Check if quest already done on this character!
            client.Message(L"The following quests are available: ");
            client.Message(quest.questName);
        }

        co_return;
    }

    concurrencpp::result<void> QuestingPlugin::UserCmdConfirmQuest(ClientId client)
    {
        auto group = client.GetGroup();
        if (group.HasError())
        {
            client.Message(L"You've not started preparing a quest yet!");
            co_return;
        }

        const auto groupId = group.Value();
        const auto quest = activeQuests.find(groupId.GetValue());
        if (quest == activeQuests.end())
        {
            client.Message(L"You've not started preparing a quest yet!");
            co_return;
        }

        client.Message(L"Quest started!");
        quest->second->questStarted = true;
        if (quest->second->stages[0].onStageBegin.has_value())
        {
            quest->second->stages[0].onStageBegin.value()();
        }

        co_return;
    }

} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

// clang-format off
constexpr auto getPi = []
{
    return PluginInfo{
        .name = L"Questing",
        .shortName = L"questing",
        .versionMajor = PluginMajorVersion::V05,
        .versionMinor = PluginMinorVersion::V00
    };
};

SetupPlugin(QuestingPlugin);
