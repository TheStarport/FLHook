#pragma once

#include "API/FLHook/Plugin.hpp"
#include "Core/Commands/AbstractUserCommandProcessor.hpp"

#include <sol/sol.hpp>

namespace Plugins
{
    /**
     * @date May 2025
     * @author Laz
     * @brief
     * This plugin provides a scriptable interface for writing custom missions and WoW-style quests for players
     */
    class QuestingPlugin final : public Plugin, public AbstractUserCommandProcessor
    {
            struct QuestStage
            {
                    std::string description;
                    sol::optional<sol::protected_function> onStageBegin;
                    sol::protected_function shouldChangeStage;
                    uint fallbackStage;
                    sol::optional<bool> missionFail;
                    // TODO: Collect stats on completed/failed quests
            };

            struct QuestInfo
            {
                    bool repeatable;
                    bool isPersistent;
                    std::wstring questName;
                    sol::optional<RepGroupId> requiredRep;
                    sol::optional<float> requiredMinRep;
                    sol::optional<float> requiredMaxRep;
                    sol::optional<int> cleanUpTime;
                    std::string luaFile;
            };

        public:
            struct QuestInstance
            {
                    QuestInfo* info;
                    sol::state lua;
                    uint currentStage = 0;
                    std::unordered_map<uint, QuestStage> stages;
                    uint timeUntilCleanup = UINT_MAX;
                    std::wstring questStarter;
                    bool questStarted = false;

                    explicit QuestInstance(QuestInfo* info) : info(info) {}
            };

        private:
            std::vector<QuestInfo> quests;
            std::unordered_map<uint, std::unique_ptr<QuestInstance>> activeQuests;

            static void SetupLuaState(sol::state* lua);

            void CurrentStageTimer();

            bool OnLoadSettings() override;
            void OnServerStartupAfter() override;
            void OnBaseEnterAfter(BaseId base, ClientId client) override;

            Task UserCmdStartQuest(ClientId client, StrToEnd questName);
            Task UserCmdListQuests(ClientId client) const;
            Task UserCmdConfirmQuest(ClientId client);

            // clang-format off
            inline static const std::array<CommandInfo<QuestingPlugin>, 3> commands =
            {
                {
                    AddCommand(QuestingPlugin, Cmds(L"/quest start"), UserCmdStartQuest, L"/quest start <quest name>", L"Start a quest with the specified"),
                    AddCommand(QuestingPlugin, Cmds(L"/quest list"), UserCmdListQuests, L"/quest list", L"List the possible quests you can start!"),
                    AddCommand(QuestingPlugin, Cmds(L"/quest confirm"), UserCmdConfirmQuest, L"/quest confirm", L"Begin a quest that has been started"),
                }
            };
            // clang-format on

            SetupUserCommandHandler(QuestingPlugin, commands);

        public:
            explicit QuestingPlugin(const PluginInfo& info);
    };
} // namespace Plugins
