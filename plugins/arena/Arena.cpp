/**
 * @date August, 2022
 * @author MadHunter (Ported by Raikkonen 2022)
 * @defgroup Arena Arena
 * @brief
 * This plugin is used to beam players to/from an arena system for the purpose of pvp.
 *
 * @paragraph cmds Player Commands
 * All commands are prefixed with '/' unless explicitly specified.
 * - arena (configurable) - This beams the player to the pvp system.
 * - return - This returns the player to their last docked base.
 *
 * @paragraph adminCmds Admin Commands
 * There are no admin commands in this plugin.
 *
 * @paragraph configuration Configuration
 * @code
 * {
 *     "command": "arena",
 *     "restrictedSystem": "Li01",
 *     "targetBase": "Li02_01_Base",
 *     "targetSystem": "Li02"
 * }
 * @endcode
 *
 * @paragraph ipc IPC Interfaces Exposed
 * This plugin does not expose any functionality.
 *
 * @paragraph optional Optional Plugin Dependencies
 * This plugin uses the "Base" plugin.
 */
#include "PCH.hpp"

#include "API/API.hpp"
#include "Arena.hpp"
#include "FLHook.hpp"

namespace Plugins
{

    /// Clear client info when a client connects.
    void Arena::ClearClientInfo(ClientId& client) { transferFlags[client] = ClientState::None; }

    /// Load the configuration
    void Arena::LoadSettings() { config = Serializer::LoadFromJson<Config>(L"config/arena.json"); }

    /** @ingroup Arena
     * @brief Returns true if the client is docked, returns false otherwise.
     */
    bool IsDockedClient(unsigned int client) { return Hk::Player::GetCurrentBase(client).Unwrap(); }

    /** @ingroup Arena
     * @brief Returns true if the client doesn't hold any commodities, returns false otherwise. This is to prevent people using the arena system as a trade
     * shortcut.
     */
    bool ValidateCargo(ClientId& client)
    {
        int holdSize = 0;

        const auto cargo = Hk::Player::EnumCargo(client, holdSize).Handle();

        for (const auto& item : cargo)
        {
            bool flag = false;
            pub::IsCommodity(item.archId, flag);

            // Some commodity present.
            if (flag)
            {
                return false;
            }
        }

        return true;
    }

    /** @ingroup Arena
     * @brief Stores the return point for the client in their save file (this should be changed).
     */
    void StoreReturnPointForClient(unsigned int client)
    {
        // It's not docked at a custom base, check for a regular base
        auto base = Hk::Player::GetCurrentBase(client).Handle();

        Hk::IniUtils::i()->SetCharacterIni(client, L"conn.retbase", std::to_wstring(base));
    }

    /** @ingroup Arena
     * @brief This returns the return base id that is stored in the client's save file.
     */
    unsigned int ReadReturnPointForClient(unsigned int client) { return StringUtils::Cast<uint>(Hk::IniUtils::i()->GetCharacterIni(client, L"conn.retbase")); }

    /** @ingroup Arena
     * @brief Move the specified client to the specified base.
     */
    void MoveClient(unsigned int client, unsigned int targetBase)
    {
        // Ask that another plugin handle the beam.
        // if (global->baseCommunicator && global->baseCommunicator->CustomBaseBeam(client, targetBase))
        //	return;

        // No plugin handled it, do it ourselves.
        SystemId system = Hk::Player::GetSystem(client).Unwrap();
        const Universe::IBase* base = Universe::get_base(targetBase);

        Hk::Player::Beam(client, targetBase);
    }

    /** @ingroup Arena
     * @brief Checks the client is in the specified base. Returns true is so, returns false otherwise.
     */
    bool CheckReturnDock(unsigned int client, unsigned int target)
    {
        if (auto base = Hk::Player::GetCurrentBase(client); base.Unwrap() == target)
        {
            return true;
        }

        return false;
    }

    /** @ingroup Arena
     * @brief Hook on CharacterSelect. Sets their transfer flag to "None".
     */
    void Arena::CharacterSelect([[maybe_unused]] const std::string& charFilename, ClientId& client) { transferFlags[client] = ClientState::None; }

    /** @ingroup Arena
     * @brief Hook on PlayerLaunch. If their transfer flags are set appropriately, redirect the undock to either the arena base or the return point
     */
    void Arena::PlayerLaunch_AFTER([[maybe_unused]] const uint& ship, ClientId& client)
    {
        if (transferFlags[client] == ClientState::Transfer)
        {
            if (!ValidateCargo(client))
            {
                PrintUserCmdText(client, cargoErrorText);
                return;
            }

            transferFlags[client] = ClientState::None;
            MoveClient(client, targetBaseId);
            return;
        }

        if (transferFlags[client] == ClientState::Return)
        {
            if (!ValidateCargo(client))
            {
                PrintUserCmdText(client, cargoErrorText);
                return;
            }

            transferFlags[client] = ClientState::None;
            const unsigned int returnPoint = ReadReturnPointForClient(client);

            if (!returnPoint)
            {
                return;
            }

            MoveClient(client, returnPoint);
            Hk::IniUtils::i()->SetCharacterIni(client, L"conn.retbase", L"0");
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // USER COMMANDS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** @ingroup Arena
     * @brief Used to switch to the arena system
     */
    void Arena::UserCmdArena()
    {
        // Prohibit jump if in a restricted system or in the target system
        if (SystemId system = Hk::Player::GetSystem(client).Unwrap(); system == restrictedSystemId || system == targetSystemId)
        {
            PrintUserCmdText(client, L"ERR Cannot use command in this system or base");
            return;
        }

        if (!IsDockedClient(client))
        {
            PrintUserCmdText(client, dockErrorText);
            return;
        }

        if (!ValidateCargo(client))
        {
            PrintUserCmdText(client, cargoErrorText);
            return;
        }

        StoreReturnPointForClient(client);
        PrintUserCmdText(client, L"Redirecting undock to Arena.");
        transferFlags[client] = ClientState::Transfer;
    }

    /** @ingroup Arena
     * @brief Used to return from the arena system.
     */
    void Arena::UserCmdReturn()
    {
        if (!ReadReturnPointForClient(client))
        {
            PrintUserCmdText(client, L"No return possible");
            return;
        }

        if (!IsDockedClient(client))
        {
            PrintUserCmdText(client, dockErrorText);
            return;
        }

        if (!CheckReturnDock(client, targetBaseId))
        {
            PrintUserCmdText(client, L"Not in correct base");
            return;
        }

        if (!ValidateCargo(client))
        {
            PrintUserCmdText(client, cargoErrorText);
            return;
        }

        PrintUserCmdText(client, L"Redirecting undock to previous base");
        transferFlags[client] = ClientState::Return;
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"Arena", L"arena", PluginMajorVersion::VERSION_04, PluginMinorVersion::VERSION_01);


Arena::Arena(const PluginInfo& info) : Plugin(info)
{
    EmplaceHook(HookedCall::FLHook__LoadSettings, &Arena::LoadSettings, HookStep::After);
    EmplaceHook(HookedCall::IServerImpl__CharacterSelect, &Arena::CharacterSelect);
    EmplaceHook(HookedCall::IServerImpl__PlayerLaunch, &Arena::PlayerLaunch_AFTER, HookStep::After);
    EmplaceHook(HookedCall::FLHook__ClearClientInfo, &Arena::ClearClientInfo, HookStep::After);
}
SetupPlugin(Arena, Info);
