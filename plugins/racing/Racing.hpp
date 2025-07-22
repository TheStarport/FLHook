#pragma once

#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{
    class RacingPlugin : public Plugin, public AbstractUserCommandProcessor
    {
            enum class Participant
            {
                Racer,
                Disqualified,
                Spectator
            };

            struct RaceArch
            {
                    std::wstring raceName;
                    Id raceStartObj;
                    uint raceNum;
                    bool loopable = false;
                    PathEntry firstWaypoint;
                    std::list<PathEntry> waypoints;
                    SystemId startingSystem;
                    std::vector<Transform> startingPositions;
                    float waypointDistance = 70.f;
            };

            struct Race;
            struct Racer
            {
                    std::wstring racerName;
                    ClientId clientId;
                    int64 newWaypointTimer;
                    std::list<PathEntry> waypoints;
                    int pool;
                    Participant participantType;
                    std::vector<mstime> completedWaypoints;
                    std::shared_ptr<Race> race;
                    int64 bestLapTime;
            };

            struct Race
            {
                    ClientId hostId;
                    uint raceId;
                    int cashPool;
                    int loopCount = 1;
                    uint highestWaypointCount = 0;
                    std::unordered_map<ClientId, std::shared_ptr<Racer>> participants;
                    RaceArch* raceArch;
                    bool started = false;
                    bool hasWinner = false;
                    int startCountdown = 0;
            };

            struct Config final
            {
                    std::unordered_map<Id, std::unordered_map<uint, RaceArch>> raceObjMap;
                    std::unordered_set<Id> racingEngines;
                    std::unordered_set<Id> bannedShips;
                    bool disruptorSuppressionPrevention = false;
            };

            struct Scoreboard
            {
                    std::unordered_map<Id, std::unordered_map<uint, std::map<float, std::wstring>>> scoreboard;
            };

            Config config;

            Scoreboard scoreboard;
            std::unordered_map<ClientId, std::shared_ptr<Racer>> racersMap;
            std::list<std::shared_ptr<Race>> raceList;

            const float DefaultWaypointDetectionDistance = 150.f;

            int raceInternalIdCounter;

            void ToggleRaceMode(ClientId client, bool newState, float waypointDistance);
            void UnfreezePlayer(ClientId client);
            void FreezePlayer(ClientId client, float time, bool instantStop);

            void SaveScoreboard();

            void NotifyPlayers(std::shared_ptr<Race> race, ClientId exceptionPlayer, std::wstring_view msg);
            int GetLapCount(std::shared_ptr<Racer> racer);
            int GetCurrentLapCheckpoints(std::shared_ptr<Racer> racer);
            void PrintRaceStatus(std::shared_ptr<Race> race);
            float GetFinishTime(std::shared_ptr<Racer> racer, mstime currTime);
            float GetLapTime(std::shared_ptr<Racer> racer, mstime currTime);
            void CheckForHighscore(std::shared_ptr<Racer> racer, float time);
            void ProcessWinner(std::shared_ptr<Racer> racer, bool isWinner, mstime currTime);
            void BeamPlayers(std::shared_ptr<Race> race, float freezeTime);
            std::shared_ptr<Racer> RegisterPlayer(std::shared_ptr<Race> race, ClientId client, int initialPool, bool spectator);
            std::shared_ptr<Race> CreateRace(ClientId hostId, RaceArch& raceArch, int initialPool, int loopCount, bool spectate);
            void DisbandRace(std::shared_ptr<Race> race, bool abruptEnd);
            void DisqualifyPlayer(ClientId client);
            void SendNextWaypoints(std::shared_ptr<Racer> racer, bool setupWaypoints);
            void BeginRace(std::shared_ptr<Race> race, int countdown);

            void OnShipExplosionHit(Ship* ship, ExplosionDamageEvent* explosion, DamageList* dmgList) override;
            void OnCharacterSelectAfter(ClientId client) override;
            void OnDisconnect(ClientId client, EFLConnection connection) override;
            void OnShipDestroy(Ship* ship, DamageList* dmgList, ShipId killerId) override;
            void OnBaseEnter(BaseId baseId, ClientId client) override;
            void OnClearClientInfo(ClientId client) override;
            bool OnLoadSettings() override;

            concurrencpp::result<void> UserCmdRaceJoin(ClientId client, std::wstring_view spectator);
            concurrencpp::result<void> UserCmdRaceSolo(ClientId client, int loopCount, uint raceNum);
            concurrencpp::result<void> UserCmdRaceHost(ClientId client, std::wstring_view spectator, std::optional<uint> raceNum);
            concurrencpp::result<void> UserCmdRaceSetLaps(ClientId client, int lapsCount);
            concurrencpp::result<void> UserCmdRaceStart(ClientId client);
            concurrencpp::result<void> UserCmdRaceWithdraw(ClientId client);
            concurrencpp::result<void> UserCmdRaceDisband(ClientId client);
            concurrencpp::result<void> UserCmdRaceInfo(ClientId client);
            concurrencpp::result<void> UserCmdRaceStatus(ClientId client);
            concurrencpp::result<void> UserCmdRaceScoreboard(ClientId client, uint raceNum);
            concurrencpp::result<void> UserCmdRaceHelp(ClientId client);

            // clang-format off
            const inline static std::array<CommandInfo<RacingPlugin>, 11> commands = {
                {
                 AddCommand(RacingPlugin, Cmds(L"/race solo"), UserCmdRaceSolo, L"/race solo <lapCount> [raceNum]", L"Demand listed amount from your current target."),
                 AddCommand(RacingPlugin, Cmds(L"/race join"), UserCmdRaceJoin, L"/race join [spectator]", L"Pays a tax request that has been issued to you."),
                 AddCommand(RacingPlugin, Cmds(L"/race setlaps"), UserCmdRaceSetLaps, L"/race setlaps <numberOfLaps>", L"Pays a tax request that has been issued to you."),
                 AddCommand(RacingPlugin, Cmds(L"/race host"), UserCmdRaceHost, L"/race host", L"Pays a tax request that has been issued to you."),
                 AddCommand(RacingPlugin, Cmds(L"/race start"), UserCmdRaceStart, L"/race start", L"Pays a tax request that has been issued to you."),
                 AddCommand(RacingPlugin, Cmds(L"/race withdraw"), UserCmdRaceWithdraw, L"/race withdraw", L"Pays a tax request that has been issued to you."),
                 AddCommand(RacingPlugin, Cmds(L"/race disband"), UserCmdRaceDisband, L"/race disband", L"Pays a tax request that has been issued to you."),
                 AddCommand(RacingPlugin, Cmds(L"/race info"), UserCmdRaceInfo, L"/race info", L"Pays a tax request that has been issued to you."),
                 AddCommand(RacingPlugin, Cmds(L"/race status"), UserCmdRaceStatus, L"/race status", L"Pays a tax request that has been issued to you."),
                 AddCommand(RacingPlugin, Cmds(L"/race scoreboard"), UserCmdRaceScoreboard, L"/race scoreboard", L"Pays a tax request that has been issued to you."),
                 AddCommand(RacingPlugin, Cmds(L"/race"), UserCmdRaceHelp, L"/race", L"Pays a tax request that has been issued to you."),
                 }
            };

            SetupUserCommandHandler(RacingPlugin, commands);
            // clang-format on

            const std::string scoreboardPath = "config/racingScoreboard.json";

        public:
            explicit RacingPlugin(const PluginInfo& info);
    };
} // namespace Plugins
