#pragma once

#include "API/FLHook/Plugin.hpp"
#include "Core/Commands/AbstractAdminCommandProcessor.hpp"

namespace Plugins
{
    /**
     * @date Jul 2025
     * @author Aingar
     * @brief
     * This plugin controls economy related logic. With clienthook support allows for separated buy/sell prices.
     * It also overrides loot-dropping logic
     */
    class MarketControllerPlugin final : public Plugin, public PacketInterface, public AbstractAdminCommandProcessor
    {

            //! Configurable fields for this plugin
            struct Config final
            {
                    // Reflectable fields
                    //! Setting this value changes behavior of loot to no longer despawn when no players are within this range
                    //! Exact mechanics are not understood but loot forcibly spawned with current tools are a lot more prone to despawning.
                    //! Leave at 0 to keep default game value (1500m)
                    float clootUnseenDespawnRange = 0.0f;

                    //! Enables all features underneath.
                    bool enableCustomLootDrops = false;

                    //! Killing an NPC with higher rep than this will produce no loot
                    float maximumRepForLoot = 0.0f;

                    //! If enabled, considers the iMin value on a commodity as the 'player sell price'
                    //! Requires a clientside override enabled for the separated buy/sell prices to be visible ingame
                    bool useSeparateBuySellPrices = false;
            };
            Config config;

            struct LootData
            {
                    uint maxDropPlayer = 5000;
                    uint maxDropNPC = 5000;
                    float dropChanceUnmounted = 1.0f;
                    float dropChanceMounted = 0.0f;
            };
            std::unordered_map<Id, LootData> lootData;

            struct CommodityLimitStruct
            {
                    std::list<std::wstring> tagRestrictions;
                    std::unordered_set<EquipmentId> allowedIds;
            };

            struct ConfigCL final
            {
                    std::unordered_map<Id, CommodityLimitStruct> mapCommodityRestrictions;
            };
            ConfigCL configCl;

            std::array<bool, MaxClientId + 1> mapBuySuppression;
            std::unordered_map<Id, std::pair<float, std::unordered_map<ushort, float>>> colGrpCargoMap;
            std::unordered_map<uint, std::unordered_map<Id, float>> cargoVolumeOverrideMap;
            std::unordered_map<uint, float> unstableJumpObjMap;
            std::unordered_map<uint, float> dropMap;

            void LoadGameData();

            bool OnLoadSettings() override;

            void OnAcceptTrade(ClientId client, bool newTradeAcceptState) override;

            bool OnSystemSwitchOutPacket(ClientId client, FLPACKET_SYSTEM_SWITCH_OUT& packet) override;
            std::optional<DOCK_HOST_RESPONSE> OnDockCall(const ShipId& shipId, const ObjectId& spaceId, int dockPortIndex,
                                                         DOCK_HOST_RESPONSE response) override;
            void OnLogin(ClientId client, const SLoginInfo& li) override;

            void ProcessPendingLootDrops();

            void OnGfGoodSell(ClientId client, const SGFGoodSellInfo& info) override;
            void OnGfGoodBuy(ClientId client, const SGFGoodBuyInfo& info) override;
            float EquipDescCommodityVolume(ClientId clientId, Id shipArch);
            float EquipDescCommodityVolume(ClientId clientId);
            void OnGfGoodBuyAfter(ClientId client, const SGFGoodBuyInfo& info) override;
            void OnRequestChangeCash(ClientId client, int newCash) override;
            void OnRequestAddItem(ClientId client, GoodId& goodId, std::wstring_view hardpoint, int count, float status, bool mounted) override;

            void OnShipDestroy(Ship* ship, DamageList* dmgList, ShipId killerId) override;
            void OnShipColGrpDestroy(Ship*, CArchGroup*, DamageEntry::SubObjFate, DamageList*) override;
            void OnShipDropAllCargo(Ship* ship, const char* hardPoint, DamageList* dmgList) override;

            TractorFailureCode OnTractorVerifyTarget(CETractor* tractor, CLoot* loot, TractorFailureCode originalValue);
            int OnGetSpaceForCargoType(CShip* ship, Archetype::Equipment* cargo) override;
            float OnGetCargoRemaining(CShip* ship) override;

            concurrencpp::result<void> AdminCmdReloadPrices(const ClientId client);

            // CommodityLimit
            bool LoadSettingsCL();
            void ClearClientInfoCL(ClientId client);
            bool GFGoodBuyCL(struct SGFGoodBuyInfo const& gbi, ClientId client);
            bool ReqAddItemCL(ClientId client);
            bool ReqChangeCashCL(ClientId client);

            // clang-format off
            inline static const std::array<AdminCommandInfo<MarketControllerPlugin>, 1> commands =
            {
                {
                    AddAdminCommand(MarketControllerPlugin, Cmds(L".reloadPrices"), AdminCmdReloadPrices, ConsoleOnly, Any, L".reloadPrices", L"Reloads price overrides from config file"),
                }
            };
            // clang-format on

            SetupAdminCommandHandler(MarketControllerPlugin, commands);

        public:
            explicit MarketControllerPlugin(const PluginInfo& info);
    };
} // namespace Plugins
