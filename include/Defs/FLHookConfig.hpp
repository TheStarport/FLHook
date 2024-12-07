#pragma once

struct DLL FLHookConfig final
{
        struct AutoKicks final
        {
                //! Amount of time spent idly on a base resulting in a server kick, in seconds.
                uint antiBaseIdle = 600;
                //! Amount of time spent idly on character select screen resulting in a server kick, in seconds.
                uint antiCharMenuIdle = 600;

                //! Number of milliseconds the player character remains in space after disconnecting.
                uint antiF1 = 0;
        };

        struct Bans final
        {
                //! If true, apply a vanilla FLServer ban in case of a wildcard/IP match on top of kicking them.
                bool banAccountOnMatch = false;
                //! Instantly kicks any player with a matching IP or matching IP range.
                std::vector<std::wstring> banWildcardsAndIPs;
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
        };

        struct ChatConfig final
        {
                struct MsgStyle final
                {
                        std::wstring msgEchoStyle = L"0x00AA0090";
                        std::wstring deathMsgStyle = L"0x19198C01";
                        std::wstring deathMsgStyleSys = L"0x1919BD01";
                        //! Time in ms between kick message rendering and actual server kick occurring.
                        uint kickMsgPeriod = 5000;
                        //! Kick message content.
                        std::wstring kickMsg = LR"(You will be kicked. Reason: {0})";
                        std::wstring userCmdStyle = L"0x00FF0090";
                        std::wstring adminCmdStyle = L"0x00FF0090";
                        //! Death message for admin kills.
                        std::wstring deathMsgTextAdminKill = L"Death: {0} was killed by an admin";
                        //! Default player to player kill message.
                        std::wstring deathMsgTextPlayerKill = L"Death: {0} was killed by {2} ({1})";
                        //! Death message for weapon self-kills.
                        std::wstring deathMsgTextSelfKill = L"Death: {0} killed himself ({1})";
                        //! Death message for player deaths to NPCs.
                        std::wstring deathMsgTextNPC = L"Death: {0} was killed by an NPC";
                        //! Death message for environmental deaths.
                        std::wstring deathMsgTextSuicide = L"Death: {0} committed suicide";
                };

                MsgStyle msgStyle;

                //! If true, chatConfig will sent only to local ships
                bool defaultLocalChat = false;

                //! If true, sends a copy of submitted commands to the player's chatlog.
                bool echoCommands = true;

                //! If true, invalid commands are not sent to the chat
                bool suppressInvalidCommands = true;

                //! Broadcasts a message that the player is attempting docking to all players in range
                //! currently hardcoded to 15K
                bool dockingMessages = true;
        };

        struct DatabaseConfig final
        {
                std::string uri = "mongodb://localhost:27017";
                std::string dbName = "FLHook";
                std::string accountsCollection = "accounts";
                std::string charactersCollection = "characters";
                std::string mailCollection = "mail";
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

                //! Maximum amount of players in a group.
                uint maxGroupSize = 8;
                //! If true, keeps the player in the group if they switch characters within one account.
                bool persistGroup = false;

                //! Global damage multiplier to missile and torpedo type weapons.
                float torpMissileBaseDamageMultiplier = 1.0f;

                bool tempBansEnabled = true;

                //! A vector of forbidden words/phrases, which will not be processed and sent to other players
                std::vector<std::wstring> chatSuppressList;
                //! Vector of systems where players can't deal damage to one another.
                std::vector<SystemId> noPvPSystems;
        };

        struct Logging final
        {
                //! If true, enables FLHook debug mode, also enabled debug level logs
                int minLogLevel = 2;

                //! If true, it logs performance of functions if they take too long to execute.
                bool logPerformanceTimers = false;
        };

        struct Npc
        {
                //! If above zero, disables NPC spawns if "server load in ms" goes above the specified value.
                //! If below zero, NPCs are always disabled
                int disableNPCSpawns = 0;

                //! The distance at which NPCs can get away from a player before despawning. Does not include NPCs spawned
                //! through the API or commands. Vanilla default is 2.5k
                float npcPersistDistance = 6500.f;

                //! The distance at which ALL NPCs will be visible from. Vanilla default is 2.5k
                float npcVisibilityDistance = 6500.f;

                //! Toggles the NPC spin protection
                bool enableNpcSpinProtection = false;

                //! The minimum amount of mass a ship must have for spin protection to kick in
                float spinProtectionMass = 180.0f;

                //! The higher this value is the more aggressive the spin protection is
                float spinImpulseMultiplier = -1.0f;
        };

        struct Plugins final
        {
                //! If true, loads all plugins on FLHook startup.
                bool loadAllPlugins = true;
                //! Contains a list of plugins to be enabled on startup if loadAllPlugins is false,
                //! or plugins to be excluded from being loaded on startup if loadAllPlugins is true.
                std::vector<std::wstring> plugins = {};
        };

        struct UserCommands final
        {
                //! A list of all commands that users cannot use. The prefix should not be included.
                std::vector<std::wstring> disabledCommands;

                //! Maximum size of users added via /ignore command
                uint userCmdMaxIgnoreList = 0;

                //! If true, the default player chat will be local, not system.
                bool defaultLocalChat = false;
        };

        struct Rename final
        {
                //! If set, will require this amount of cash for every rename
                int renameCost = 0;

                //! If set, prevents renaming of the same character for specified number of days;
                int cooldown = 0;
        };

        struct Reputatation final
        {
                //! IF set, causes /droprep command to cost the defined amount of credits to go through.
                int creditCost;
        };

        struct HttpSettings final
        {
                bool enableHttpServer = true;
                std::string host = "localhost";
                int port = 5577;
                int timeout = 5;
                int maxPayloadSize = 1024 * 1024 * 2; // Default 2MB
                bool sendJsonInsteadOfBson = false;
        };

        Logging logging;
        General general;
        AutoKicks autoKicks;
        Plugins plugins;
        UserCommands userCommands;
        Bans bans;
        ChatConfig chatConfig;
        Callsign callsign;
        DatabaseConfig database;
        Npc npc;
        Rename rename;
        Reputatation reputatation;
        HttpSettings httpSettings;
};
