#pragma once



namespace Plugins
{
    /**
     * @date Feb, 2010
     * @author Cannon, ported by Raikkonen, Nen and Laz
     * @brief
     * The Purchase Restrictions plugin restricts the purchase of equipment, goods and ships unless the player holds a certain item.
     *
     * @par Configuration
     * @code
     * {
     *    "checkItemRestrictions": false,
     *    "enforceItemRestrictions": false,
     *    "goodItemRestrictions": {
     *        "li_gun01_mark02": [
     *            "li_gun01_mark03"
     *        ]
     *    },
     *    "goodPurchaseDenied": "You are not authorized to buy this item.",
     *    "itemsOfInterest": [
     *        "li_gun01_mark01"
     *    ],
     *    "shipItemRestrictions": {
     *        "li_fighter": [
     *            "li_gun01_mark03"
     *        ]
     *    },
     *    "shipPurchaseDenied": "You are not authorized to buy this ship.",
     *    "unbuyableItems": [
     *        "li_gun01_mark01"
     *    ]
     * }
     * @endcode
     *
     * @par Player Commands
     * There are no user commands in this plugin.
     *
     * @par adminCmds Admin Commands
     * There are no admin commands in this plugin.
     */
    class PurchaseRestrictionsPlugin final : public Plugin
    {
            //! Config data for this plugin
            struct Config
            {
                    //! Check whether a player is trying to buy items without the correct id
                    bool checkItemRestrictions = false;

                    //! Block them for buying said item without the correct id
                    bool enforceItemRestrictions = false;

                    //! Messages when they are blocked from buying something
                    std::wstring shipPurchaseDenied = L"You are not authorized to buy this ship.";
                    std::wstring goodPurchaseDenied = L"You are not authorized to buy this item.";

                    //! Items that we log transfers for
                    std::vector<GoodId> itemsOfInterest{};

                    //! Items that cannot be bought at all.
                    std::vector<GoodId> unbuyableItems{};

                    //! Items that can only be bought with a certain item equipped (item, [ equippedItemsThatAllowPurchase ])
                    std::unordered_map<GoodId, std::vector<GoodId>> goodItemRestrictions{};

                    //! Ships that can only be bought with a certain item equipped (ship, [ equippedItemsThatAllowPurchase ])
                    std::unordered_map<GoodId, std::vector<GoodId>> shipItemRestrictions{};
            };

            Config config;
            ReturnCode returnCode = ReturnCode::Default;
            std::unordered_map<ClientId, bool> clientSuppressBuy;

            void LogItemsOfInterest(const ClientId& client, GoodId goodId, std::wstring_view details);
            bool CheckIdEquipRestrictions(ClientId client, GoodId goodId, bool isShip) const;
            bool OnLoadSettings() override;
            void OnClearClientInfo(ClientId client) override;
            void OnPlayerLaunch(ClientId client, ShipId ship) override;
            void OnBaseEnter(BaseId base, ClientId client) override;
            void OnGfGoodBuy(ClientId client, const SGFGoodBuyInfo& info) override;
            void OnRequestAddItem(ClientId client, GoodId goodId, std::wstring_view hardpoint, int count, float status, bool mounted) override;
            void OnRequestChangeCash(ClientId client, int cash) override;
            void OnRequestSetCash(ClientId client, int cash) override;
            void OnRequestEquipment(ClientId client, const EquipDescList& edl) override;
            void OnRequestShipArch(ClientId client, ArchId arch) override;
            void OnRequestHullStatus(ClientId client, float status) override;

        public:
            explicit PurchaseRestrictionsPlugin(const PluginInfo& info);
    };
} // namespace Plugins
