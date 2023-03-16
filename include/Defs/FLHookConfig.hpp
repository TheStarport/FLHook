#pragma once

#include "Tools/Serialization/Serializer.hpp"

struct DLL FLHookConfig final : Reflectable, Singleton<FLHookConfig>
{
	std::string File() override;

	struct General final : Reflectable
	{
		//! Time of invulnerability upon undock in milliseconds.
		//! Protected player can also not inflict any damage.
		uint antiDockKill = 4000;
		//! Number of milliseconds the player character remains in space after disconnecting.
		uint antiF1 = 0;
		//! Disable the cruise disrupting effects.
		bool changeCruiseDisruptorBehaviour = false;
		//! If true, enables FLHook debug mode.
		bool debugMode = false;

		//! If true, it encodes player characters to bini (binary ini)
		bool disableCharfileEncryption = false;
		//! If a player disconnects in space, their ship will remain in game world for the time specified, in milliseconds.
		uint disconnectDelay = 0;
		//! If above zero, disables NPC spawns if "server load in ms" goes above the specified value.
		uint disableNPCSpawns = 0;

		//! If true, it uses local time when rendering current time instead of server time,
		//! in for example, "/time" function.
		bool localTime = false;
		//! Maximum amount of players in a group.
		uint maxGroupSize = 8;
		//! NOT IMPLEMENTED YET: if true, keeps the player in the group if they switch characters within one account.
		bool persistGroup = false;
		//! Number of slots reserved to specified accounts.
		uint reservedSlots = 0;
		//! Global damage multiplier to missile and torpedo type weapons.
		float torpMissileBaseDamageMultiplier = 1.0f;
		//! If true, it logs performance of functions if they take too long to execute.
		bool logPerformanceTimers = false;

		bool tempBansEnabled = true;

		//! A vector of forbidden words/phrases, which will not be processed and sent to other players
		std::vector<std::wstring> chatSuppressList;
		//! Vector of systems where players can't deal damage to one another.
		std::vector<std::string> noPVPSystems;

		std::vector<uint> noPVPSystemsHashed;

		//! Amount of time spent idly on a base resulting in a server kick, in seconds.
		uint antiBaseIdle = 600;
		//! Amount of time spent idly on character select screen resulting in a server kick, in seconds.
		uint antiCharMenuIdle = 600;
	};

	struct Plugins final : Reflectable
	{
		//! If true, loads all plugins on FLHook startup.
		bool loadAllPlugins = true;
		//! Contains a list of plugins to be enabled on startup if loadAllPlugins is false,
		//! or plugins to be excluded from being loaded on startup if loadAllPlugins is true.
		std::vector<std::string> plugins = {};
	};

	struct MsgStyle final : Reflectable
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
	};

	struct Message final : Reflectable
	{
		MsgStyle msgStyle;

		//! If true, messages will sent only to local ships
		bool defaultLocalChat = false;

		//! If true, sends a copy of submitted commands to the player's chatlog.
		bool echoCommands = true;

		//! If true, invalid commands are not echo'ed
		bool suppressInvalidCommands = true;

		//! If true, player's death renders the default message.
		bool dieMsg = true;

		//! Broadcasts a message that the player is attempting docking to all players in range
		//! currently hardcoded to 15K
		bool dockingMessages = true;
	};

	struct UserCommands final : Reflectable
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
	};

	struct Bans final : Reflectable
	{
		//! If true, apply a vanilla FLServer ban in case of a wildcard/IP match on top of kicking them.
		bool banAccountOnMatch = false;
		//! Instantly kicks any player with a matching IP or matching IP range.
		std::vector<std::wstring> banWildcardsAndIPs;
	};

	struct Callsign final : Reflectable
	{
		//! The mapping of numbers to formations. 1, min = Alpha. 29, max = Yanagi
		std::vector<int> allowedFormations = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29};

		//! If true, formations and numbers will not be assigned to ships. All ships will be alpha 1-1.
		bool disableRandomisedFormations = false;

		//! If true, NPCs will refer to all players as freelancer
		bool disableUsingAffiliationForCallsign = false;
	};

	General general;
	Plugins plugins;
	UserCommands userCommands;
	Bans bans;
	Message messages;
	Callsign callsign;
};
