#pragma once

#include "Core/Commands/AbstractUserCommandProcessor.hpp"

namespace Plugins
{
    /**
     * @date July, 2022
     * @author Nekura Mew
     * @defgroup Tax
     * @brief The Tax plugin allows players to issue 'formally' make credit demands and declare hostilities.
     *
     * @par Configuration
     * @code
     * {
     *     "cannotPay": "This rogue isn't interested in money. Run for cover, they want to kill you!",
     *     "customColor": 9498256,
     *     "customFormat": 144,
     *     "huntingMessage": "You are being hunted by %s. Run for cover, they want to kill you!",
     *     "huntingMessageOriginator": "Good luck hunting %s !",
     *     "killDisconnectingPlayers": true,
     *     "maxTax": 300,
     *     "minplaytimeSec": 0,
     *     "taxRequestReceived": "You have received a tax request: Pay %d credits to %s! type \"/pay\" to pay the tax."
     * }
     * @endcode
     *
     * @par Player Commands
     * -tax <amount> - demand listed amount from the player, for amount equal zero, it declares hostilities.
     * -pay - submits the demanded payment to the issuing player
     *
     * @par Admin Commands
     * None
     *
     * @note All player commands are prefixed with '/', all admin commands are prefixed with a '.'
     */
    class TaxPlugin final : public Plugin, public AbstractUserCommandProcessor
    {
            std::shared_ptr<Timer> timer;

            struct Config final
            {
                    //! Maximum amount of credits a player can demand from another.
                    uint maxTax = 300;
                    //! Color of messages that will be broadcasted by this plugin.
                    MessageColor customColor = MessageColor::LightGreen;
                    //! Formatting of the messages broadcasted by this plugin.
                    MessageFormat customFormat = MessageFormat::Small;
                    //! Message letting the target know about the size of the demand, as well as informing them on how to comply.
                    std::wstring taxRequestReceived = L"You have received a tax request: Pay {} credits to {}! Type \"/pay\" to pay the tax.";
                    //! Message letting the target know they're being attacked.
                    std::wstring huntingMessage = L"You are being hunted by {}. Run for cover, they want to kill you!";
                    //! Confirmation message to the aggressor that the victim has been informed.
                    std::wstring huntingMessageOriginator = L"Good luck hunting {} !";
                    //! Message received if payment attempt is made on
                    std::wstring cannotPay = L"This rogue isn't interested in money. Run for cover, they want to kill you!";
                    //! If true, kills the players who disconnect while having a demand levied against them.
                    bool killDisconnectingPlayers = true;
            };

            //! Structs
            struct Tax
            {
                    ClientId targetId;
                    ClientId initiatorId;
                    std::wstring target{};
                    std::wstring initiator{};
                    uint cash = 0;
                    bool f1 = false;
            };

            void RemoveTax(const Tax& toRemove);
            void UserCmdTax(std::wstring_view taxAmount);
            void UserCmdPay();
            void FiveSecondTimer();

            void OnDisconnect(ClientId client, EFLConnection connection) override;
            void OnLoadSettings() override;

            Config config;
            std::list<Tax> taxes{};
            std::vector<uint> excludedsystemsIds{};

            const inline static std::array<CommandInfo<TaxPlugin>, 2> commands = {
                {
                 AddCommand(TaxPlugin, Cmds(L"/tax"), UserCmdTax, L"/tax <amount>", L"Demand listed amount from your current target."),
                 AddCommand(TaxPlugin, Cmds(L"/pay"), UserCmdPay, L"/pay", L"Pays a tax request that has been issued to you."),
                 }
            }; // namespace Plugins

            SetupUserCommandHandler(TaxPlugin, commands);

        public:
            explicit TaxPlugin(const PluginInfo& info);
            ~TaxPlugin() override;
    };
}