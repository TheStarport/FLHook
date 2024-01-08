#pragma once

#include "API/Utils/Serializer.hpp"

struct DLL FLHookConfig final : Singleton<FLHookConfig>
{
        struct Debug final
        {
                //! If true, enables FLHook debug mode, also enabled debug level logs
                bool debugMode = false;

                //! If true, any log statement made at a 'Trace', will be allowed.
                bool logTraceLevel = false;

                //! If true, it logs performance of functions if they take too long to execute.
                bool logPerformanceTimers = false;

                NLOHMANN_DEFINE_TYPE_INTRUSIVE(Debug, debugMode, logTraceLevel, logPerformanceTimers);
        };

        struct General final
        {
                //! Time of invulnerability upon undock in milliseconds.
                //! Protected player can also not inflict any damage.
                uint antiDockKill = 4000;

                //! The type of damage allowed on this server. Allowed values: 'All', 'None', 'PvP' and 'PvE'
                DamageMode damageMode = DamageMode::All;

                //! Disable the cruise disrupting effects.
                bool changeCruiseDisruptorBehaviour = false;

                //! If true, it encodes player characters to bini (binary ini)
                bool disableCharfileEncryption = false;
                //! If a player disconnects in space, their ship will remain in game world for the time specified, in milliseconds.
                uint disconnectDelay = 0;
                //! If above zero, disables NPC spawns if "server load in ms" goes above the specified value.
                uint disableNPCSpawns = 0;

                //! Maximum amount of players in a group.
                uint maxGroupSize = 8;
                //! NOT IMPLEMENTED YET: if true, keeps the player in the group if they switch characters within one account.
                bool persistGroup = false;

                //! Global damage multiplier to missile and torpedo type weapons.
                float torpMissileBaseDamageMultiplier = 1.0f;

                bool tempBansEnabled = true;

                //! A vector of forbidden words/phrases, which will not be processed and sent to other players
                std::vector<std::wstring> chatSuppressList;
                //! Vector of systems where players can't deal damage to one another.
                std::vector<std::wstring> noPVPSystems;

                std::vector<uint> noPVPSystemsHashed;

                Serialize(General, antiDockKill, changeCruiseDisruptorBehaviour, damageMode, disableCharfileEncryption, disconnectDelay, disableNPCSpawns,
                          maxGroupSize, persistGroup, torpMissileBaseDamageMultiplier, tempBansEnabled, chatSuppressList, noPVPSystems);
        };

        struct AutoKicks final
        {
                //! Amount of time spent idly on a base resulting in a server kick, in seconds.
                uint antiBaseIdle = 600;
                //! Amount of time spent idly on character select screen resulting in a server kick, in seconds.
                uint antiCharMenuIdle = 600;

                //! Number of milliseconds the player character remains in space after disconnecting.
                uint antiF1 = 0;

                Serialize(AutoKicks, antiBaseIdle, antiCharMenuIdle, antiF1)
        };

        struct Plugins final
        {
                //! If true, loads all plugins on FLHook startup.
                bool loadAllPlugins = true;
                //! Contains a list of plugins to be enabled on startup if loadAllPlugins is false,
                //! or plugins to be excluded from being loaded on startup if loadAllPlugins is true.
                std::vector<std::wstring> plugins = {};

                Serialize(Plugins, loadAllPlugins, plugins);
        };

        struct MsgStyle final
        {
                std::wstring msgEchoStyle = L"0x00AA0090";
                std::wstring deathMsgStyle = L"0x19198C01";
                std::wstring deathMsgStyleSys = L"0x1919BD01";
                //! Time in ms between kick message rendering and actual server kick occurring.
                uint kickMsgPeriod = 5000;
                //! Kick message content.
                std::wstring kickMsg = LR"(<TRA data=" 0x0000FF10 " mask=" - 1 "/><TEXT>You will be kicked. Reason: %reason</TEXT>)";
                std::wstring userCmdStyle = L"0x00FF0090";
                std::wstring adminCmdStyle = L"0x00FF0090";
                //! Death message for admin kills.
                std::wstring deathMsgTextAdminKill = L"Death: %victim was killed by an admin";
                //! Default player to player kill message.
                std::wstring deathMsgTextPlayerKill = L"Death: %victim was killed by %killer (%type)";
                //! Death message for weapon self-kills.
                std::wstring deathMsgTextSelfKill = L"Death: %victim killed himself (%type)";
                //! Death message for player deaths to NPCs.
                std::wstring deathMsgTextNPC = L"Death: %victim was killed by an NPC";
                //! Death message for environmental deaths.
                std::wstring deathMsgTextSuicide = L"Death: %victim committed suicide";

                Serialize(MsgStyle, msgEchoStyle, deathMsgStyle, deathMsgStyleSys, kickMsgPeriod, kickMsg, userCmdStyle, adminCmdStyle, deathMsgTextAdminKill,
                          deathMsgTextPlayerKill, deathMsgTextSelfKill, deathMsgTextNPC, deathMsgTextSuicide);
        };

        struct MessageQueue final
        {
                //! Enable FLHook Message Queues - See setup: https://docs.flhook.org/
                bool enableQueues = true;

                //! The hostname of your RabbitMQ instance
                std::wstring hostName = L"localhost";

                //! The port of your RabbitMQ instance
                int port = 5672;

                //! The username to connect to RabbitMQ
                std::wstring username = L"guest";

                //! The password to connect to RabbitMQ
                std::wstring password = L"guest";

                //! If true FLHook will communicate with RabbitMQ over AMPQS, using a SSL connection.
                bool ensureSecureConnection = false;

                Serialize(MessageQueue, enableQueues, hostName, port, username, password, ensureSecureConnection);
        };

        struct ChatConfig final
        {
                MsgStyle msgStyle;

                //! If true, chatConfig will sent only to local ships
                bool defaultLocalChat = false;

                //! If true, sends a copy of submitted commands to the player's chatlog.
                bool echoCommands = true;

                //! If true, invalid commands are not sent to the chat
                bool suppressInvalidCommands = true;

                //! If true, player's death renders the default message.
                bool dieMsg = true;

                //! Broadcasts a message that the player is attempting docking to all players in range
                //! currently hardcoded to 15K
                bool dockingMessages = true;

                Serialize(ChatConfig, msgStyle, defaultLocalChat, echoCommands, suppressInvalidCommands, dieMsg, dockingMessages);
        };

        struct UserCommands final
        {
                //! Can users use SetDieMsgSize command
                bool userCmdSetDieMsgSize = true;
                //! Can users use SetDieMsg command
                bool userCmdSetDieMsg = true;
                //! Can users use SetChatFont command
                bool userCmdSetChatFont = true;
                //! Can users use Ignore command
                bool userCmdIgnore = true;
                //! Can users use Help command
                bool userCmdHelp = true;
                //! Maximum size of users added via /ignore command
                uint userCmdMaxIgnoreList = 0;
                //! If true, the default player chat will be local, not system.
                bool defaultLocalChat = false;

                Serialize(UserCommands, userCmdSetDieMsgSize, userCmdSetDieMsg, userCmdSetChatFont, userCmdIgnore, userCmdHelp, userCmdMaxIgnoreList,
                          defaultLocalChat);
        };

        struct Bans final
        {
                //! If true, apply a vanilla FLServer ban in case of a wildcard/IP match on top of kicking them.
                bool banAccountOnMatch = false;
                //! Instantly kicks any player with a matching IP or matching IP range.
                std::vector<std::wstring> banWildcardsAndIPs;

                Serialize(Bans, banAccountOnMatch, banWildcardsAndIPs);
        };

        struct Callsign final
        {
                //! The mapping of numbers to formations. 1, min = Alpha. 29, max = Yanagi
                std::vector<AllowedFormation> allowedFormations = {
                    AllowedFormation::Alpha,   AllowedFormation::Beta,   AllowedFormation::Gamma,  AllowedFormation::Delta, AllowedFormation::Epsilon,
                    AllowedFormation::Zeta,    AllowedFormation::Theta,  AllowedFormation::Iota,   AllowedFormation::Kappa, AllowedFormation::Lambda,
                    AllowedFormation::Omicron, AllowedFormation::Sigma,  AllowedFormation::Omega,  AllowedFormation::Red,   AllowedFormation::Blue,
                    AllowedFormation::Gold,    AllowedFormation::Green,  AllowedFormation::Silver, AllowedFormation::Black, AllowedFormation::White,
                    AllowedFormation::Yellow,  AllowedFormation::Matsu,  AllowedFormation::Sakura, AllowedFormation::Fuji,  AllowedFormation::Botan,
                    AllowedFormation::Hagi,    AllowedFormation::Susuki, AllowedFormation::Kiku,   AllowedFormation::Yanagi
                };

                //! If true, formations and numbers will not be assigned to ships. All ships will be alpha 1-1.
                bool disableRandomisedFormations = false;

                //! If true, NPCs will refer to all players as freelancer
                bool disableUsingAffiliationForCallsign = false;

                Serialize(Callsign, allowedFormations, disableRandomisedFormations, disableUsingAffiliationForCallsign);
        };

        struct DatabaseConfig final
        {
                std::string uri;

                Serialize(DatabaseConfig, uri);
        };

        Debug debug;
        General general;
        AutoKicks autoKicks;
        Plugins plugins;
        MessageQueue messageQueue;
        UserCommands userCommands;
        Bans bans;
        ChatConfig chatConfig;
        Callsign callsign;
        DatabaseConfig databaseConfig;

        Serialize(FLHookConfig, debug, general, autoKicks, plugins, messageQueue, userCommands, bans, chatConfig, callsign, databaseConfig);
};
