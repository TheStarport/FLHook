#include "PCH.hpp"

#include "API/FLHook/ResourceManager.hpp"
#include "API/Utils/LuaHelper.hpp"
#include "Questing.hpp"

namespace Plugins
{
    void QuestingPlugin::SetupLuaState(sol::state* lua)
    {
        LuaHelper::InitialiseDefaultLuaState(lua);

        // clang-format off
        lua->new_usertype<QuestStage>("QuestStage", sol::constructors<QuestStage()>(),
            "description", &QuestStage::description,
            "on_stage_begin", &QuestStage::onStageBegin,
            "should_change_stage", &QuestStage::shouldChangeStage,
            "fallback_stage", &QuestStage::fallbackStage,
            "mission_fail", &QuestStage::missionFail
        );

        lua->new_usertype<QuestInfo>("QuestInfo",
            "repeatable", &QuestInfo::repeatable,
            "is_persistent", &QuestInfo::isPersistent,
            "quest_name", &QuestInfo::questName,
            "required_rep", &QuestInfo::requiredRep,
            "required_min_rep", &QuestInfo::requiredMinRep,
            "required_max_rep", &QuestInfo::requiredMaxRep
        );
        // clang-format on
    }
} // namespace Plugins
