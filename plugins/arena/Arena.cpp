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

#include "Arena.hpp"

namespace Plugins
{
    ArenaPlugin::ArenaPlugin(const PluginInfo& info) : Plugin(info) {}

    /// Clear client info when a client connects.
    void ArenaPlugin::OnClearClientInfo(const ClientId client)
    {
        transferFlags[client.GetValue()] = ClientState::None;
    }

    /// Load the configuration
    void ArenaPlugin::OnLoadSettings()
    {
        if (const auto conf = Json::Load<Config>("config/arena.json"); !conf.has_value())
        {
            Json::Save(config, "config/arena.json");
        }
        else
        {
            config = conf.value();
        }
    }

    /** @ingroup Arena
     * @brief Returns true if the client doesn't hold any commodities, returns false otherwise. This is to prevent people using the arena system as a trade
     * shortcut.
     */
    bool ArenaPlugin::ValidateCargo(ClientId client)
    {
        int remainingHoldSize;
        for (const auto cargo = client.EnumCargo(remainingHoldSize).Handle(); const auto& item : cargo)
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
    void ArenaPlugin::StoreReturnPointForClient(ClientId client)
    {
        //auto base = client.GetCurrentBase().Handle();
        // TODO: Save in DB
        // Hk::IniUtils::i()->SetCharacterIni(client, L"conn.retbase", std::to_wstring(base));
    }

    /** @ingroup Arena
     * @brief This returns the return base id that is stored in the client's save file.
     */
    BaseId ArenaPlugin::ReadReturnPointForClient(ClientId client)
    {
        // TODO: Read from DB
        // TODO: Validate base in DB still exists
        // return StringUtils::Cast<uint>(Hk::IniUtils::i()->GetCharacterIni(client, L"conn.retbase"));
        return BaseId();
    }

    /** @ingroup Arena
     * @brief Hook on CharacterSelect. Sets their transfer flag to "None".
     */
    void ArenaPlugin::OnCharacterSelect(const ClientId client, std::wstring_view charFilename)
    {
        transferFlags[client.GetValue()] = ClientState::None;
    }

    /** @ingroup Arena
     * @brief Hook on PlayerLaunch. If their transfer flags are set appropriately, redirect the undock to either the arena base or the return point
     */
    void ArenaPlugin::OnPlayerLaunchAfter(ClientId client, ShipId ship)
    {
        auto state = transferFlags[client.GetValue()];
        if (state == ClientState::Transfer)
        {
            if (!ValidateCargo(client))
            {
                (void)client.Message(cargoErrorText);
                return;
            }

            transferFlags[client.GetValue()] = ClientState::None;
            (void)client.Beam(targetBaseId);
            return;
        }

        if (state == ClientState::Return)
        {
            if (!ValidateCargo(client))
            {
                (void)client.Message(cargoErrorText);
                return;
            }

            transferFlags[client.GetValue()] = ClientState::None;
            const BaseId returnPoint = ReadReturnPointForClient(client);

            if (!returnPoint)
            {
                return;
            }

            (void)client.Beam(returnPoint);
            // TODO: Unset in DB
            // Hk::IniUtils::i()->SetCharacterIni(client, L"conn.retbase", L"0");
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // USER COMMANDS
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////

    /** @ingroup Arena
     * @brief Used to switch to the arena system
     */
    void ArenaPlugin::UserCmdArena()
    {
        // Prohibit jump if in a restricted system or in the target system
        if (const SystemId system = userCmdClient.GetSystemId().Unwrap();
            std::ranges::find(restrictedSystems, system) != restrictedSystems.end() || system == targetSystemId)
        {
            (void)userCmdClient.Message(L"ERR Cannot use command in this system or base");
            return;
        }

        if (!userCmdClient.IsDocked())
        {
            (void)userCmdClient.Message(dockErrorText);
            return;
        }

        if (!ValidateCargo(userCmdClient))
        {
            (void)userCmdClient.Message(cargoErrorText);
            return;
        }

        StoreReturnPointForClient(userCmdClient);
        (void)userCmdClient.Message(L"Redirecting undock to Arena.");
        transferFlags[userCmdClient.GetValue()] = ClientState::Transfer;
    }

    /** @ingroup Arena
     * @brief Used to return from the arena system.
     */
    void ArenaPlugin::UserCmdReturn()
    {
        if (!ReadReturnPointForClient(userCmdClient))
        {
            (void)userCmdClient.Message(L"No return possible");
            return;
        }

        if (!userCmdClient.IsDocked())
        {
            (void)userCmdClient.Message(dockErrorText);
            return;
        }

        if (userCmdClient.GetCurrentBase().Unwrap() != targetBaseId)
        {
            (void)userCmdClient.Message(L"Not in correct base");
            return;
        }

        if (!ValidateCargo(userCmdClient))
        {
            (void)userCmdClient.Message(cargoErrorText);
            return;
        }

        (void)userCmdClient.Message(L"Redirecting undock to previous base");
        transferFlags[userCmdClient.GetValue()] = ClientState::Return;
    }
} // namespace Plugins

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"Arena", L"arena", PluginMajorVersion::V04, PluginMinorVersion::V01);
SetupPlugin(ArenaPlugin, Info);
