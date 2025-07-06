#pragma once

class PersonalityHelper
{
        friend FLHook;

        std::unordered_map<std::wstring, pub::AI::Personality> pilots;
        std::unordered_map<std::wstring, pub::AI::Personality::EvadeDodgeUseStruct> evadeDodge;
        std::unordered_map<std::wstring, pub::AI::Personality::EvadeBreakUseStruct> evadeBreak;
        std::unordered_map<std::wstring, pub::AI::Personality::BuzzHeadTowardUseStruct> buzzHead;
        std::unordered_map<std::wstring, pub::AI::Personality::BuzzPassByUseStruct> buzzPass;
        std::unordered_map<std::wstring, pub::AI::Personality::TrailUseStruct> trail;
        std::unordered_map<std::wstring, pub::AI::Personality::StrafeUseStruct> strafe;
        std::unordered_map<std::wstring, pub::AI::Personality::EngineKillUseStruct> engineKill;
        std::unordered_map<std::wstring, pub::AI::Personality::RepairUseStruct> repair;
        std::unordered_map<std::wstring, pub::AI::Personality::GunUseStruct> gun;
        std::unordered_map<std::wstring, pub::AI::Personality::MissileUseStruct> missile;
        std::unordered_map<std::wstring, pub::AI::Personality::MineUseStruct> mine;
        std::unordered_map<std::wstring, pub::AI::Personality::MissileReactionStruct> missileReaction;
        std::unordered_map<std::wstring, pub::AI::Personality::DamageReactionStruct> damageReaction;
        std::unordered_map<std::wstring, pub::AI::Personality::CountermeasureUseStruct> cm;
        std::unordered_map<std::wstring, pub::AI::Personality::FormationUseStruct> formation;
        std::unordered_map<std::wstring, pub::AI::Personality::JobStruct> job;

        Action<pub::AI::Personality*> GetPersonality(const std::wstring& pilotNickname);
        static void SetDirection(INI_Reader& ini, float (&direction)[4]);

        void LoadEvadeDodge(INI_Reader& ini);
        void LoadEvadeBreak(INI_Reader& ini);
        void LoadBuzzHead(INI_Reader& ini);
        void LoadBuzzPass(INI_Reader& ini);
        void LoadTrail(INI_Reader& ini);
        void LoadStrafe(INI_Reader& ini);
        void LoadEngineKill(INI_Reader& ini);
        void LoadRepair(INI_Reader& ini);
        void LoadGun(INI_Reader& ini);
        void LoadMine(INI_Reader& ini);
        void LoadMissileReaction(INI_Reader& ini);
        void LoadDamageReaction(INI_Reader& ini);
        void LoadCM(INI_Reader& ini);
        void LoadFormation(INI_Reader& ini);
        static void GetDifficulty(INI_Reader& ini, int& difficulty);
        void LoadJob(INI_Reader& ini);
        void LoadMissile(INI_Reader& ini);
        void LoadPilot(INI_Reader& ini);

    public:
        PersonalityHelper();
        PersonalityHelper(const PersonalityHelper&) = delete;
        PersonalityHelper& operator=(const PersonalityHelper&) = delete;
        ~PersonalityHelper() = default;

        static pub::AI::SetPersonalityParams MakePersonality();
};
