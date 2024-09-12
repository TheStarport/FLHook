#include "PCH.hpp"

#include "API/FLHook/ClientList.hpp"
#include "Tax.hpp"

using namespace Plugins;

void TaxPlugin::RemoveTax(const Tax& toRemove)
{
    const auto taxToRemove =
        std::ranges::find_if(taxes, [&toRemove](const Tax& tax) { return tax.targetId == toRemove.targetId && tax.initiatorId == toRemove.initiatorId; });
    taxes.erase(taxToRemove);
}

Task TaxPlugin::UserCmdTax(ClientId client, const std::wstring_view taxAmount)
{
    const auto& noPvpSystems = FLHook::GetConfig()->general.noPvPSystems;
    // no-pvp check
    if (const SystemId system = client.GetSystemId().Unwrap(); std::ranges::find(noPvpSystems, system) == noPvpSystems.end())
    {
        client.Message(L"Error: You cannot tax in a No-PvP system.");
        co_return TaskStatus::Finished;
    }

    if (taxAmount.empty())
    {
        client.Message(L"Usage:");
        client.Message(L"/tax <credits>");
        co_return TaskStatus::Finished;
    }

    const uint taxValue = StringUtils::MultiplyUIntBySuffix(taxAmount);

    if (taxValue > config.maxTax)
    {
        client.Message(std::format(L"Error: Maximum tax value is {} credits.", config.maxTax));
        co_return TaskStatus::Finished;
    }

    const auto targetShip = client.GetShip().Handle().GetTarget().Handle();

    const auto player = targetShip.GetPlayer().Unwrap();
    if (!player)
    {
        client.Message(L"Error: Your current target is not a player.");
        co_return TaskStatus::Finished;
    }

    for (const auto& [targetId, initiatorId, target, initiator, cash, f1] : taxes)
    {
        if (targetId == player)
        {
            client.Message(L"Error: There already is a tax request pending for this player.");
            co_return TaskStatus::Finished;
        }
    }

    Tax tax;
    tax.initiatorId = client;
    tax.targetId = player;
    tax.cash = taxValue;
    taxes.push_back(tax);

    const auto characterName = client.GetCharacterName().Handle();

    if (taxValue == 0)
    {
        player.Message(std::vformat(config.huntingMessage, std::make_wformat_args(characterName)), config.customFormat, config.customColor);
    }
    else
    {
        player.Message(std::vformat(config.taxRequestReceived, std::make_wformat_args(taxValue, characterName)), config.customFormat, config.customColor);
    }

    const auto targetCharacterName = player.GetCharacterName().Handle();

    // send confirmation msg
    if (taxValue > 0)
    {
        client.Message(std::format(L"Tax request of {} credits sent to {}!", taxValue, targetCharacterName));
    }
    else
    {
        client.Message(std::vformat(config.huntingMessageOriginator, std::make_wformat_args(targetCharacterName)));
    }

    co_return TaskStatus::Finished;
}

Task TaxPlugin::UserCmdPay(const ClientId client)
{
    for (auto& it : taxes)
    {
        if (it.targetId != client)
        {
            continue;
        }

        if (it.cash == 0)
        {
            client.Message(config.cannotPay);
            co_return TaskStatus::Finished;
        }

        if (const auto cash = client.GetCash().Unwrap(); cash < it.cash)
        {
            client.Message(L"You have not enough money to pay the tax.");
            it.initiatorId.Message(L"The player does not have enough money to pay the tax.");
            co_return TaskStatus::Finished;
        }

        client.RemoveCash(it.cash).Handle();
        it.initiatorId.AddCash(it.cash).Handle();
        client.Message(L"You paid the tax.");

        const auto characterName = client.GetCharacterName().Handle();
        it.initiatorId.Message(std::format(L"{} paid the tax!", characterName));
        RemoveTax(it);

        // Queue up some saves
        client.SaveChar();
        it.initiatorId.SaveChar();
        co_return TaskStatus::Finished;
    }

    client.Message(L"Error: No tax request was found that could be accepted!");

    co_return TaskStatus::Finished;
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

            if (data.shipId && config.killDisconnectingPlayers)
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

bool TaxPlugin::OnLoadSettings()
{
    LoadJsonWithValidation(Config, config, "config/tax.json");

    return true;
}

TaxPlugin::TaxPlugin(const PluginInfo& info) : Plugin(info)
{
    timer = Timer::Add([this] { FiveSecondTimer(); }, 5000);
}
TaxPlugin::~TaxPlugin() { Timer::Remove(timer); }

DefaultDllMain();

const PluginInfo Info(L"Tax", L"tax", PluginMajorVersion::V05, PluginMinorVersion::V00);
SetupPlugin(TaxPlugin, Info);
