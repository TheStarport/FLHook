#include "PCH.hpp"

#include "Racing.hpp"

namespace Plugins
{
    // TODO: Implement fluf clienthooks
    void RacingPlugin::ToggleRaceMode(ClientId client, bool newState, float waypointDistance)
    {
        if (!waypointDistance)
        {
            waypointDistance = DefaultWaypointDetectionDistance;
        }
    }
    void RacingPlugin::UnfreezePlayer(ClientId client) {}
    void RacingPlugin::FreezePlayer(ClientId client, float time, bool instantStop) {}
    void RacingPlugin::SaveScoreboard()
    {
        Json::Save(scoreboard, scoreboardPath);
    }

    concurrencpp::result<void> RacingPlugin::UserCmdRaceHelp(ClientId client)
    {
        client.Message(L"/race solo <lapCount> [raceNum]");
        client.Message(L"/race host [spectator]");
        client.Message(L"/race join [spectator]");
        client.Message(L"/race setlaps <lapCount>");
        client.Message(L"/race start");
        client.Message(L"/race disband");
        client.Message(L"/race withdraw");
        client.Message(L"/race status");
        client.Message(L"/race info");

        co_return;
    }

    concurrencpp::result<void> RacingPlugin::UserCmdRaceScoreboard(ClientId client, uint raceNum)
    {
        if (!client.InSpace())
        {
            co_return;
        }

        auto target = client.GetShip().Handle().GetTarget().Unwrap();
        if (!target)
        {
            client.MessageErr(L"No target!");
            co_return;
        }

        auto raceObjIter = config.raceObjMap.find(target.GetId().Handle());
        if (raceObjIter == config.raceObjMap.end())
        {
            client.MessageErr(L"ERR Invalid start point object selected!");
            co_return;
        }

        auto raceIter = raceObjIter->second.find(raceNum);
        if (raceIter == raceObjIter->second.end())
        {
            if (raceObjIter->second.size() == 1)
            {
                raceIter = raceObjIter->second.begin();
            }
            else
            {
                client.MessageErr(L"Invalid race number for selected start! Available races:");
                for (auto& race : raceObjIter->second)
                {
                    client.Message(std::format(L"{} - {}", race.second.raceNum, race.second.raceName));
                }
                co_return;
            }
        }

        auto scoreboardIter = scoreboard.scoreboard.find(target.GetId().Handle());
        if (scoreboardIter == scoreboard.scoreboard.end())
        {
            client.Message(L"Scoreboard for this race is empty!");
            co_return;
        }

        auto raceScoreIter = scoreboardIter->second.find(raceIter->second.raceNum);
        if (raceScoreIter == scoreboardIter->second.end())
        {
            client.Message(L"Scoreboard for this race is empty!");
            co_return;
        }

        uint counter = 0;
        client.Message(std::format(L"Scoreboard for {}:", raceIter->second.raceName));
        for (auto& entry : raceScoreIter->second)
        {
            client.Message(std::format(L"#{} {:.3f}s - {}", ++counter, entry.first, entry.second));
        }
        co_return;
    }

    concurrencpp::result<void> RacingPlugin::UserCmdRaceStatus(ClientId client)
    {
        auto regIter = racersMap.find(client);
        if (regIter == racersMap.end())
        {
            client.MessageErr(L"Not registered in a race");
            co_return;
        }
        if (!regIter->second->race->started)
        {
            client.MessageErr(L"Race hasn't started yet");
            co_return;
        }
        PrintRaceStatus(regIter->second->race);

        co_return;
    }

    concurrencpp::result<void> RacingPlugin::UserCmdRaceInfo(ClientId client)
    {
        auto regIter = racersMap.find(client);
        if (regIter == racersMap.end())
        {
            client.MessageErr(L"Not registered in a race");
            co_return;
        }

        auto& race = regIter->second->race;
        client.Message(std::format(L"Host: {}", client.GetCharacterName().Handle()));

        client.Message(std::format(L"Track: {}", race->raceArch->raceName));
        client.Message(std::format(L"Laps: {}", race->loopCount));

        client.Message(std::format(L"Your contribution: ${} credits", regIter->second->pool));
        client.Message(L"Participants:");
        for (auto& participant : race->participants)
        {
            if (participant.second->participantType == Participant::Racer)
            {
                client.Message(std::format(L"- {}", participant.second->racerName));
            }
        }

        co_return;
    }

    concurrencpp::result<void> RacingPlugin::UserCmdRaceDisband(ClientId client)
    {
        auto regIter = racersMap.find(client);
        if (regIter == racersMap.end())
        {
            client.MessageErr(L"Not registered in a race");
            co_return;
        }
        auto& race = regIter->second->race;
        if (client != race->hostId)
        {
            client.MessageErr(L"Not registered in a race");
            co_return;
        }

        if (race->started)
        {
            client.MessageErr(L"Not registered in a race");
            co_return;
        }

        DisbandRace(race, true);
        co_return;
    }

    concurrencpp::result<void> RacingPlugin::UserCmdRaceWithdraw(ClientId client)
    {
        auto regIter = racersMap.find(client);
        if (regIter == racersMap.end())
        {
            client.MessageErr(L"Not registered in a race");
            co_return;
        }
        auto& racer = regIter->second;
        auto& race = regIter->second->race;

        if (!race->started && racer->pool)
        {
            client.AddCash(racer->pool);
            race->cashPool -= racer->pool;
            client.Message(L"Withdrawn from the race, credits refunded");

            NotifyPlayers(race,
                          client,
                          std::format(L"{} has withdrawn from the race, race credit pool reduced to {}", client.GetCharacterName().Handle(), race->cashPool));
        }
        else
        {
            client.Message(L"Withdrawn from the race");
            NotifyPlayers(race, client, std::format(L"{} has withdrawn from the race", client.GetCharacterName().Handle()));
        }

        if (racer->participantType == Participant::Racer)
        {
            racer->participantType = Participant::Disqualified;
        }
        else if (racer->participantType == Participant::Spectator)
        {
            racer->race->participants.erase(racer->clientId);
        }
        ToggleRaceMode(client, false, 0.f);
        racersMap.erase(regIter);

        co_return;
    }

    concurrencpp::result<void> RacingPlugin::UserCmdRaceStart(ClientId client)
    {
        auto regIter = racersMap.find(client);
        if (regIter == racersMap.end())
        {
            client.MessageErr(L"Not hosting a race!");
            co_return;
        }

        if (regIter->second->race->hostId != client)
        {
            client.MessageErr(L"Not hosting a race!");
            co_return;
        }

        for (auto& racer : regIter->second->race->participants)
        {
            if (racer.second->participantType == Participant::Racer)
            {
                BeginRace(regIter->second->race, 10);
                co_return;
            }
        }

        client.MessageErr(L"No racer participants in race!");

        co_return;
    }

    concurrencpp::result<void> RacingPlugin::UserCmdRaceHost(ClientId client, std::wstring_view spectator, std::optional<uint> raceNum)
    {
        if (client.InSpace())
        {
            client.MessageErr(L"Not in space!");
            co_return;
        }

        auto ship = client.GetShip().Handle();
        auto target = ship.GetTarget().Unwrap();
        if (!target)
        {
            client.MessageErr(L"No target selected!");
            co_return;
        }

        if (racersMap.count(client))
        {
            client.MessageErr(L"You are already registered in a race");
            co_return;
        }

        auto raceObjIter = config.raceObjMap.find(target.GetId().Handle());
        if (raceObjIter == config.raceObjMap.end())
        {
            client.MessageErr(L"Invalid start point object selected!");
            co_return;
        }

        if (Vector::Distance(target.GetPosition().Handle(), ship.GetPosition().Handle()) > 5000.f)
        {
            client.MessageErr(L"Too far from the start object!");
            co_return;
        }

        auto raceIter = raceObjIter->second.find(raceNum.value_or(0));
        if (raceIter == raceObjIter->second.end())
        {
            if (raceObjIter->second.size() == 1)
            {
                raceIter = raceObjIter->second.begin();
            }
            else
            {
                client.MessageErr(L"Invalid race number for selected start! Available races:");
                for (auto& race : raceObjIter->second)
                {
                    client.Message(std::format(L"{} - {}", race.second.raceNum, race.second.raceName.c_str()));
                }
                co_return;
            }
        }

        bool spectate = spectator.find(L"spec") != std::wstring::npos;

        CreateRace(client, raceIter->second, 0, 1, spectate);

        raceIter->second.startingSystem.Message(std::format(L"New race: {} started by {}!", raceIter->second.raceName, client.GetCharacterName().Handle()));

        co_return;
    }

    concurrencpp::result<void> RacingPlugin::UserCmdRaceSetLaps(ClientId client, int newLapCount)
    {
        if (client.InSpace())
        {
            client.MessageErr(L"Not in space!");
            co_return;
        }

        auto regIter = racersMap.find(client);
        if (regIter == racersMap.end())
        {
            client.MessageErr(L"ERR: Not registered to a race!");
            co_return;
        }

        auto& race = regIter->second->race;
        if (race->hostId != client)
        {
            client.MessageErr(L"ERR You're not hosting a race!");
            co_return;
        }

        if (!race->raceArch->loopable)
        {
            client.MessageErr(L"ERR This race is not a loop, it cannot have laps set!");
            co_return;
        }

        if (newLapCount <= 0)
        {
            client.MessageErr(L"ERR Invalid input!");
            co_return;
        }

        race->loopCount = newLapCount;

        NotifyPlayers(race, {}, std::format(L"Race loop count changed to {}!", newLapCount));

        if (newLapCount > 20)
        {
            NotifyPlayers(race, {}, L"Are you SURE about that?");
        }

        co_return;
    }

    concurrencpp::result<void> RacingPlugin::UserCmdRaceJoin(ClientId client, std::wstring_view spectator)
    {
        bool isSpectator = spectator.find(L"spec") == 0;

        if (!client.InSpace())
        {
            client.MessageErr(L"Not in space!");
            co_return;
        }

        auto ship = client.GetShip().Handle();
        if (!ship)
        {
            client.MessageErr(L"Not in space!");
            co_return;
        }

        auto target = ship.GetTarget().Unwrap();
        if (!target)
        {
            client.MessageErr(L"ERR No target!");
            co_return;
        }

        ClientId targetClientId = target.GetPlayer().Unwrap();
        if (!targetClientId)
        {
            client.MessageErr(L"ERR Target not a player!");
            co_return;
        }

        auto raceData = racersMap.find(targetClientId);
        if (raceData == racersMap.end())
        {
            client.MessageErr(L"ERR Target not in a race!");
            co_return;
        }

        auto newRacer = RegisterPlayer(raceData->second->race, client, 0, isSpectator);

        if (!newRacer)
        {
            co_return;
        }

        static wchar_t buf[100];
        if (isSpectator)
        {
            NotifyPlayers(raceData->second->race, client, std::format(L"{} is now spectating the race!", newRacer->racerName.c_str()));
        }
        else
        {
            NotifyPlayers(raceData->second->race, client, std::format(L"{} has joined the race!", newRacer->racerName.c_str()));
        }

        co_return;
    }

    concurrencpp::result<void> RacingPlugin::UserCmdRaceSolo(ClientId client, int loopCount, uint raceNum)
    {
        if (!client.InSpace())
        {
            client.MessageErr(L"Not in space!");
            co_return;
        }

        auto cship = client.GetShip().Handle();
        if (!cship)
        {
            client.MessageErr(L"Not in space!");
            co_return;
        }

        auto target = cship.GetTarget().Unwrap();
        if (!target)
        {
            client.MessageErr(L"No target!");
            co_return;
        }

        auto raceObjIter = config.raceObjMap.find(target.GetId().Handle());
        if (raceObjIter == config.raceObjMap.end())
        {
            client.MessageErr(L"Invalid start point object selected!");
            co_return;
        }

        if (Vector::Distance(target.GetPosition().Handle(), cship.GetPosition().Handle()) > 5000.f)
        {
            client.MessageErr(L"Too far from the start object!");
            co_return;
        }

        auto raceIter = raceObjIter->second.find(raceNum);
        if (raceIter == raceObjIter->second.end())
        {
            if (raceObjIter->second.size() == 1)
            {
                raceIter = raceObjIter->second.begin();
            }
            else
            {
                client.MessageErr(L"ERR Invalid race number for selected start! Available races:");
                for (auto& race : raceObjIter->second)
                {
                    client.Message(std::format(L"{} - {}", race.second.raceNum, race.second.raceName));
                }
                co_return;
            }
        }

        if (loopCount == 0)
        {
            if (raceIter->second.loopable)
            {
                client.MessageErr(L"ERR Invalid loop count!");
                client.MessageErr(L"Usage: /race solo <lapCount> [raceNum]");
                co_return;
            }
            loopCount = 1;
        }

        if (racersMap.count(client))
        {
            client.MessageErr(L"ERR Already in a race!");
            co_return;
        }

        auto race = CreateRace(client, raceIter->second, 0, loopCount, false);
        BeginRace(race, 5);

        co_return;
    }

    void RacingPlugin::NotifyPlayers(std::shared_ptr<Race> race, ClientId exceptionPlayer, std::wstring_view msg)
    {
        for (auto& participant : race->participants)
        {
            if (participant.first == exceptionPlayer)
            {
                continue;
            }
            if (participant.second->participantType == Participant::Disqualified)
            {
                continue;
            }
            participant.first.Message(msg);
        }
    }

    int RacingPlugin::GetLapCount(std::shared_ptr<Racer> racer)
    {
        if (racer->completedWaypoints.empty())
        {
            return 0;
        }
        return (racer->completedWaypoints.size() - 1) / racer->race->raceArch->waypoints.size();
    }

    int RacingPlugin::GetCurrentLapCheckpoints(std::shared_ptr<Racer> racer)
    {
        if (racer->completedWaypoints.empty())
        {
            return 0;
        }
        return (racer->completedWaypoints.size() - 1) % racer->race->raceArch->waypoints.size();
    }

    void RacingPlugin::PrintRaceStatus(std::shared_ptr<Race> race)
    {
        std::map<int, std::shared_ptr<Racer>> racerSpeedMap;
        float bestLapTime = FLT_MAX;

        for (auto& racer : race->participants)
        {
            switch (racer.second->participantType)
            {
                case Participant::Racer:
                    racerSpeedMap[racer.second->completedWaypoints.size()] = racer.second;
                    if (racer.second->bestLapTime != 0.0f && racer.second->bestLapTime < bestLapTime)
                    {
                        bestLapTime = racer.second->bestLapTime;
                    }
                    break;
                case Participant::Disqualified:
                    racerSpeedMap[INT32_MIN + racer.second->completedWaypoints.size()] = racer.second;
                    if (racer.second->bestLapTime != 0.0f && racer.second->bestLapTime < bestLapTime)
                    {
                        bestLapTime = racer.second->bestLapTime;
                    }
                    break;
                default:;
            }
        }

        int counter = 1;
        for (auto racerIter = racerSpeedMap.rbegin(); racerIter != racerSpeedMap.rend(); racerIter++)
        {
            const auto& racer = racerIter->second;
            static wchar_t buf[200];
            buf[0] = L'\x0';
            int lapCount = GetLapCount(racer);

            std::wstringstream str;

            str << L"#" << counter++ << L" " << racer->racerName.c_str() << L" - ";

            if (lapCount == race->loopCount)
            {
                str << L"FINISHED!";
            }
            else
            {
                str << lapCount << " laps, " << GetCurrentLapCheckpoints(racer) << L"/" << racer->race->raceArch->waypoints.size() << " checkpoints";
            }

            if (lapCount)
            {
                str.setf(std::ios_base::fixed);
                str.precision(3);
                str << L" best lap time: " << racer->bestLapTime << "s";
            }

            if (bestLapTime == racer->bestLapTime)
            {
                str << L"(best in race!)";
            }

            if (racer->participantType == Participant::Disqualified)
            {
                str << L" - DISQUALIFIED";
            }

            NotifyPlayers(race, {}, str.view());
        }
    }

    float RacingPlugin::GetFinishTime(std::shared_ptr<Racer> racer, mstime currTime)
    {
        float timeDiff = static_cast<float>(currTime - *racer->completedWaypoints.begin()) / 1000;

        return timeDiff;
    }

    float RacingPlugin::GetLapTime(std::shared_ptr<Racer> racer, mstime currTime)
    {
        uint lapSize = racer->race->raceArch->waypoints.size();
        uint currentElem = racer->completedWaypoints.size();
        mstime lapStartTime = racer->completedWaypoints.at(currentElem - lapSize - 1);

        float lapTime = static_cast<float>(currTime - lapStartTime) / 1000;

        return lapTime;
    }

    void RacingPlugin::CheckForHighscore(std::shared_ptr<Racer> racer, float time)
    {
        auto& raceArch = racer->race->raceArch;
        auto& trackScoreboard = scoreboard.scoreboard[raceArch->raceStartObj][raceArch->raceNum];

        constexpr uint SIZE = 20;

        if (config.bannedShips.contains(racer->clientId.GetShipArch().Handle().GetId()))
        {
            return;
        }

        if (!(trackScoreboard.size() < SIZE || trackScoreboard.rbegin()->first > time))
        {
            return;
        }

        auto iter = trackScoreboard.begin();
        for (; iter != trackScoreboard.end(); iter++)
        {
            if (iter->second != racer->racerName)
            {
                continue;
            }

            if (iter->first > time)
            {
                trackScoreboard.erase(iter);
                trackScoreboard[time] = racer->racerName;
                iter = trackScoreboard.begin();
                break;
            }
            return;
        }

        if (iter == trackScoreboard.end())
        {
            if (trackScoreboard.size() == SIZE)
            {
                trackScoreboard.erase(std::prev(trackScoreboard.end()));
            }

            trackScoreboard[time] = racer->racerName;
        }

        SaveScoreboard();

        auto distance = std::distance(trackScoreboard.begin(), trackScoreboard.find(time));

        ClientId().Message(std::format(L"{} has scored a new highscore on {} leaderboard: #{} - {:.3f}s",
                                       racer->racerName,
                                       racer->race->raceArch->raceName,
                                       distance + 1,
                                       time));
    }

    void RacingPlugin::ProcessWinner(std::shared_ptr<Racer> racer, bool isWinner, mstime currTime)
    {
        racer->waypoints.clear();
        if (isWinner)
        {
            racer->clientId.Message(std::format(L"You've won the \"{}\" race, {} lap(s) with time of {:.3f}s!",
                                                racer->race->raceArch->raceName,
                                                racer->race->loopCount,
                                                GetFinishTime(racer, currTime)));
            int cashPool = racer->race->cashPool;
            if (cashPool)
            {
                racer->clientId.AddCash(racer->race->cashPool);
                racer->clientId.Message(std::format(L"Rewarded ${} credits!", racer->race->cashPool));
            }

            NotifyPlayers(racer->race,
                          racer->clientId,
                          std::format(L"{} has won the race with time of {:.3f}s!", racer->racerName.c_str(), GetFinishTime(racer, currTime)));

            return;
        }

        racer->clientId.Message(std::format(L"You've finished the \"{}\" race, {} lap(s) with time of {:.3f}s!",
                                            racer->race->raceArch->raceName.c_str(),
                                            racer->race->loopCount,
                                            GetFinishTime(racer, currTime)));
    }

    void RacingPlugin::BeamPlayers(std::shared_ptr<Race> race, float freezeTime)
    {
        uint positionIndex = 0;
        auto& positionIter = race->raceArch->startingPositions;

        for (auto& participant : race->participants)
        {
            if (participant.second->participantType != Participant::Racer)
            {
                continue;
            }

            ClientId player = participant.first;
            if (player.GetSystemId().Handle() != race->raceArch->startingSystem)
            {
                player.Message(L"ERR You're in the wrong system! Unable to beam you into position");
                continue;
            }
            if (race->raceArch->startingPositions.size() == positionIndex)
            {
                player.Message(L"ERR Unable to find a starting position for you, contact admins/developers!");
                continue;
            }
            auto& startPos = race->raceArch->startingPositions.at(positionIndex++);
            player.GetShip().Handle().Relocate(startPos.pos, startPos.orient);

            player.Message(L"Race beginning, beaming you into position!");

            if (config.bannedShips.contains(player.GetShipArch().Handle().GetId()))
            {
                player.Message(L"Notice: Your ship is banned from being placed on the scoreboard!");
            }

            if (freezeTime)
            {
                FreezePlayer(player, freezeTime, true);
            }

            ToggleRaceMode(player, true, race->raceArch->waypointDistance);
        }
    }

    std::shared_ptr<RacingPlugin::Racer> RacingPlugin::RegisterPlayer(std::shared_ptr<Race> race, ClientId client, int initialPool, bool spectator)
    {
        if (race->started && !spectator)
        {
            client.Message(L"ERR this race is already underway, you can join as a spectator");
            return nullptr;
        }

        initialPool = std::max(0, initialPool);

        if (racersMap.count(client))
        {
            client.Message(L"ERR You are already registered to a race");
            return nullptr;
        }

        if (initialPool)
        {
            if (client.GetCash().Handle() < initialPool)
            {
                client.Message(L"ERR You don't have that much cash!");
                return nullptr;
            }

            client.Message(std::format(L"Added {} to the race reward pool!", initialPool));
            client.RemoveCash(initialPool);
        }

        std::shared_ptr<Racer> racer = std::make_shared<Racer>();
        racer->clientId = client;
        racer->participantType = spectator ? Participant::Spectator : Participant::Racer;
        racer->pool = initialPool;
        race->cashPool += initialPool;
        racer->racerName = client.GetCharacterName().Handle();
        racer->race = race;

        race->participants[client] = racer;

        if (!spectator)
        {
            client.Message(L"Added to the race as racer!");
        }
        else
        {
            client.Message(L"Added to the race as a spectator!");
        }

        racersMap[client] = racer;

        return racer;
    }

    std::shared_ptr<RacingPlugin::Race> RacingPlugin::CreateRace(ClientId hostId, RaceArch& raceArch, int initialPool, int loopCount, bool spectate)
    {
        if (loopCount == 0)
        {
            loopCount = 1;
        }

        Race race;

        race.hostId = hostId;
        race.raceId = ++raceInternalIdCounter;
        race.raceArch = &raceArch;
        race.loopCount = loopCount;
        race.cashPool = 0;

        std::shared_ptr<Race> racePtr = std::make_shared<Race>(race);
        raceList.push_back(racePtr);
        RegisterPlayer(racePtr, hostId, initialPool, spectate);

        return racePtr;
    }

    void RacingPlugin::DisbandRace(std::shared_ptr<Race> race, bool abruptEnd)
    {
        if (abruptEnd && !race->started)
        {
            for (auto& participant : race->participants)
            {
                if (participant.second->participantType == Participant::Disqualified)
                {
                    continue;
                }
                int poolEntry = participant.second->pool;
                if (poolEntry)
                {
                    participant.first.Message(std::format(L"Race disbanded, ${} credits refunded", poolEntry));
                    participant.first.AddCash(poolEntry);
                }
                ToggleRaceMode(participant.first, false, 300.f);
                racersMap.erase(participant.first);
            }
        }

        for (auto iter = raceList.begin(); iter != raceList.end(); ++iter)
        {
            if (iter->get()->raceId == race->raceId)
            {
                raceList.erase(iter);
                break;
            }
        }
    }

    void RacingPlugin::DisqualifyPlayer(ClientId client)
    {
        auto regIter = racersMap.find(client);
        if (regIter == racersMap.end())
        {
            return;
        }

        auto& racer = regIter->second;
        racer->participantType = Participant::Disqualified;
        if (!racer->race->started)
        {

            client.AddCash(racer->pool);
            racer->race->cashPool -= racer->pool;

            if (racer->pool)
            {
                NotifyPlayers(racer->race,
                              racer->clientId,
                              std::format(L"{} has withdrawn from the race, race credit pool reduced to {}", racer->racerName, racer->race->cashPool));
            }
            else
            {
                NotifyPlayers(racer->race, racer->clientId, std::format(L"{} has withdrawn from the race", racer->racerName));
            }
        }

        auto& race = racer->race;

        ToggleRaceMode(client, false, 300.f);
        racersMap.erase(client);

        for (auto& participant : race->participants)
        {
            if (participant.second->participantType == Participant::Racer)
            {
                return;
            }
        }

        DisbandRace(race, true);
    }

    void RacingPlugin::SendNextWaypoints(std::shared_ptr<Racer> racer, bool setupWaypoints)
    {
        if (setupWaypoints)
        {
            racer->waypoints.push_back(racer->race->raceArch->firstWaypoint);
            for (int i = 0; i < racer->race->loopCount; ++i)
            {
                racer->waypoints.insert(racer->waypoints.end(), racer->race->raceArch->waypoints.begin(), racer->race->raceArch->waypoints.end());
            }
        }

        if (racer->waypoints.empty())
        {
            return;
        }

        static RequestPath<2> sendStruct;
        sendStruct.noPathFound = false;
        sendStruct.repId = racer->clientId.GetReputation().Handle();
        sendStruct.waypointCount = 2;

        auto wpIter = racer->waypoints.begin();
        if (racer->waypoints.size() == 1)
        {
            sendStruct.pathEntries[0] = *wpIter; // send the same waypoint twice to prevent it not being parented to the final ring properly, weird bug.
            sendStruct.pathEntries[1] = *wpIter;
        }
        else
        {
            sendStruct.pathEntries[0] = *wpIter++;
            sendStruct.pathEntries[1] = *wpIter;
        }

        pub::Player::ReturnBestPath(racer->clientId.GetValue(), (uchar*)&sendStruct, 12 + (sendStruct.waypointCount * 20));
    }

    void RacingPlugin::BeginRace(std::shared_ptr<Race> race, int countdown)
    {
        for (auto& participant : race->participants)
        {
            SendNextWaypoints(participant.second, true);
        }

        BeamPlayers(race, static_cast<float>(countdown));
        race->started = true;
        race->startCountdown = countdown;
    }

    void RacingPlugin::OnShipExplosionHit(Ship* ship, ExplosionDamageEvent* explosion, DamageList* dmgList)
    {
        if (config.disruptorSuppressionPrevention)
        {
            return;
        }

        ClientId clientId = ClientId(ship->cship()->ownerPlayer);
        if (!clientId || dmgList->damageCause != DamageCause::CruiseDisrupter || !dmgList->inflictorPlayerId)
        {
            return;
        }

        auto racer = racersMap.find(clientId);
        if (racer != racersMap.end() && racer->second->race->started && racersMap.contains(ClientId(dmgList->inflictorPlayerId)))
        {
            dmgList->damageCause = DamageCause::MissileTorpedo;
            return;
        }

        CShip* cship = ship->cship();

        CEquip* engine = cship->equipManager.FindFirst((uint)EquipmentClass::Engine);
        if (!engine)
        {
            return;
        }

        if (config.racingEngines.contains(engine->archetype->archId))
        {
            FreezePlayer(clientId, 8.0f, false);
        }
    }

    void RacingPlugin::OnCharacterSelectAfter(ClientId client) { DisqualifyPlayer(client); }
    void RacingPlugin::OnDisconnect(ClientId client, EFLConnection connection) { DisqualifyPlayer(client); }
    void RacingPlugin::OnShipDestroy(Ship* ship, DamageList* dmgList, ShipId killerId)
    {
        auto client = ClientId(ship->cship()->ownerPlayer);
        if (client)
        {
            DisqualifyPlayer(client);
        }
    }
    void RacingPlugin::OnBaseEnter(BaseId baseId, ClientId client) { DisqualifyPlayer(client); }
    void RacingPlugin::OnClearClientInfo(ClientId client) { DisqualifyPlayer(client); }
    bool RacingPlugin::OnLoadSettings()
    {
        LoadJsonWithValidation(Config, config, "config/racing.json");
        LoadJsonWithValidation(Scoreboard, scoreboard, scoreboardPath);
        return true;
    }
    RacingPlugin::RacingPlugin(const PluginInfo& info) : Plugin(info) {}
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();
// clang-format off
constexpr auto getPi = []
{
	return PluginInfo{
	    .name = L"Racing",
	    .shortName = L"racing",
	    .versionMajor = PluginMajorVersion::V05,
	    .versionMinor = PluginMinorVersion::V00
	};
};

SetupPlugin(RacingPlugin);
