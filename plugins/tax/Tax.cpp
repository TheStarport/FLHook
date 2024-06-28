#include "PCH.hpp"

#include "Tax.hpp"
#include "API/FLHook/ClientList.hpp"

using namespace Plugins;

void TaxPlugin::RemoveTax(const Tax& toRemove)
{
    const auto taxToRemove =
        std::ranges::find_if(taxes, [&toRemove](const Tax& tax) { return tax.targetId == toRemove.targetId && tax.initiatorId == toRemove.initiatorId; });
    taxes.erase(taxToRemove);
}

void TaxPlugin::UserCmdTax(const std::wstring_view taxAmount)
{
    const auto& noPvpSystems = FLHook::GetConfig().general.noPvPSystems;
    // no-pvp check
    if (const SystemId system = userCmdClient.GetSystemId().Unwrap(); std::ranges::find(noPvpSystems, system) == noPvpSystems.end())
    {
        userCmdClient.Message(L"Error: You cannot tax in a No-PvP system.");
        return;
    }

    if (taxAmount.empty())
    {
        userCmdClient.Message(L"Usage:");
        userCmdClient.Message(L"/tax <credits>");
        return;
    }

    const uint taxValue = StringUtils::MultiplyUIntBySuffix(taxAmount);

    if (taxValue > config.maxTax)
    {
        userCmdClient.Message(std::format(L"Error: Maximum tax value is {} credits.", config.maxTax));
        return;
    }

    const auto targetShip = userCmdClient.GetShipId().Handle().GetTarget();

    if (!targetShip.has_value())
    {
        userCmdClient.Message(L"Error: You do not have a target selected.");
        return;
    }

    const auto player = targetShip->GetPlayer();
    if (!player.has_value())
    {
        userCmdClient.Message(L"Error: Your current target is not a player.");
    }

    for (const auto& [targetId, initiatorId, target, initiator, cash, f1] : taxes)
    {
        if (targetId == player.value())
        {
            userCmdClient.Message(L"Error: There already is a tax request pending for this player.");
            return;
        }
    }

    Tax tax;
    tax.initiatorId = userCmdClient;
    tax.targetId = player.value();
    tax.cash = taxValue;
    taxes.push_back(tax);

    const auto characterName = userCmdClient.GetCharacterName().Handle();

    if (taxValue == 0)
    {
        player->Message(std::vformat(config.huntingMessage, std::make_wformat_args(characterName)),
            config.customFormat, config.customColor);
    }
    else
    {
        player->Message(std::vformat(config.taxRequestReceived, std::make_wformat_args(taxValue, characterName)),
            config.customFormat, config.customColor);
    }

    const auto targetCharacterName = player->GetCharacterName().Handle();

    // send confirmation msg
    if (taxValue > 0)
    {
        userCmdClient.Message(std::format(L"Tax request of {} credits sent to {}!", taxValue, targetCharacterName));
    }
    else
    {
        userCmdClient.Message(std::vformat(config.huntingMessageOriginator, std::make_wformat_args(targetCharacterName)));
    }
}

void TaxPlugin::UserCmdPay()
{
    for (auto& it : taxes)
    {
        if (it.targetId != userCmdClient)
        {
            continue;
        }

        if (it.cash == 0)
        {
            userCmdClient.Message(config.cannotPay);
            return;
        }

        if (const auto cash = userCmdClient.GetCash().Unwrap(); cash < it.cash)
        {
            userCmdClient.Message(L"You have not enough money to pay the tax.");
            it.initiatorId.Message(L"The player does not have enough money to pay the tax.");
            return;
        }

        userCmdClient.RemoveCash(it.cash).Handle();
        it.initiatorId.AddCash(it.cash).Handle();
        userCmdClient.Message(L"You paid the tax.");

        const auto characterName = userCmdClient.GetCharacterName().Handle();
        it.initiatorId.Message(std::format(L"{} paid the tax!", characterName));
        RemoveTax(it);

        // Queue up some saves
        userCmdClient.SaveChar();
        it.initiatorId.SaveChar();
        return;
    }

    userCmdClient.Message(L"Error: No tax request was found that could be accepted!");
}

void TaxPlugin::FiveSecondTimer()
{
    auto currentTime = TimeUtils::UnixTime<std::chrono::milliseconds>();
    for (auto& data : FLHook::Clients())
    {
        if (data.timeDisconnect)
        {
            continue;
        }

        if (data.f1Time >= currentTime)
        {
            continue;
        }

        for (const auto& it : taxes)
        {
            if (it.targetId != data.id)
            {
                continue;
            }

            if (data.ship && config.killDisconnectingPlayers)
            {
                data.ship.Destroy(DestroyType::Fuse);
            }

            const auto characterName = it.targetId.GetCharacterName().Handle();
            it.initiatorId.Message(std::format(L"Target disconnected. Tax request to {} aborted.", characterName));
            RemoveTax(it);
            break;
        }
    }
}

void TaxPlugin::OnDisconnect([[maybe_unused]] ClientId client, [[maybe_unused]] const EFLConnection state) { FiveSecondTimer(); }

void TaxPlugin::OnLoadSettings()
{
    if (const auto conf = Json::Load<Config>("config/tax.json"); !conf.has_value())
    {
        Json::Save(config, "config/tax.json");
    }
    else
    {
        config = conf.value();
    }
}

TaxPlugin::TaxPlugin(const PluginInfo& info) : Plugin(info)
{
    timer = Timer::Add([this] { FiveSecondTimer(); }, 5000);
}
TaxPlugin::~TaxPlugin() { Timer::Remove(timer); }

DefaultDllMain();

const PluginInfo Info(L"Tax", L"tax", PluginMajorVersion::V05, PluginMinorVersion::V00);
SetupPlugin(TaxPlugin, Info);