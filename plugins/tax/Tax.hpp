#pragma once

#include "API/API.hpp"

class TaxPlugin : public Plugin, public AbstractUserCommandProcessor
{
        struct Config final
        {
                //! Minimal playtime in seconds required for a character to be a valid demand target.
                int minplaytimeSec = 0;
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

                Serialize(Config, minplaytimeSec, maxTax, customColor, customFormat, taxRequestReceived, huntingMessage, huntingMessageOriginator, cannotPay,
                          killDisconnectingPlayers);
        };

        //! Structs
        struct Tax
        {
                uint targetId = 0;
                uint initiatorId = 0;
                std::wstring target{};
                std::wstring initiator{};
                uint cash = 0;
                bool f1 = false;
        };

        void RemoveTax(const Tax& toRemove);
        void UserCmdTax(std::wstring_view taxAmount);
        void UserCmdPay();
        void TimerF1Check();
        void DisConnect(ClientId& client, const EFLConnection& state);
        void LoadSettings();

        std::unique_ptr<Config> config;
        std::list<Tax> taxes{};
        std::vector<uint> excludedsystemsIds{};

        constexpr inline static std::array<CommandInfo<TaxPlugin>, 2> commands = {
            {
             AddCommand(TaxPlugin, L"/tax", UserCmdTax, L"/tax <amount>", L"Demand listed amount from your current target."),
             AddCommand(TaxPlugin, L"/pay", UserCmdPay, L"/pay", L"Pays a tax request that has been issued to you."),
             }
        }; // namespace Plugins

        SetupUserCommandHandler(TaxPlugin, commands);

    public:
        explicit TaxPlugin(const PluginInfo& info);
};
