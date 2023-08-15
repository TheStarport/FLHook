#include "PCH.hpp"

#include "API/Utils/IniUtils.hpp"

#include <Global.hpp>
#include <Utils/Detour.hpp>

using SendCommType = int(__cdecl*)(uint, uint, uint, const Costume*, uint, uint*, int, uint, float, bool);
const std::unique_ptr<FunctionDetour<SendCommType>> func = std::make_unique<FunctionDetour<SendCommType>>(pub::SpaceObj::SendComm);

int SendComm(uint fromShipId, uint toShipId, uint voiceId, const Costume* costume, uint infocardId, uint* a5, int a6, uint infocardId2, float a8, bool a9)
{
    if (const CShip* ship = dynamic_cast<CShip*>(CObject::Find(toShipId, CObject::Class::CSHIP_OBJECT)); ship && ship->is_player())
    {
        const auto client = ship->GetOwnerPlayer();

        const auto& ci = ClientInfo[client];
        const auto* conf = FLHookConfig::c();

        static std::array<byte, 8> num1RewriteBytes = { 0xBA, 0x00, 0x00, 0x00, 0x00, 0x90, 0x90, 0x90 };

        const auto content = DWORD(GetModuleHandle(L"content.dll"));
        constexpr DWORD factionOffset = 0x6fb632c + 18 - 0x6ea0000;
        constexpr DWORD numberOffset1 = 0x6eeb49b - 0x6ea0000;
        constexpr DWORD numberOffset2 = 0x6eeb523 + 1 - 0x6ea0000;
        constexpr DWORD formationOffset = 0x6fb7524 + 25 - 0x6ea0000;

        const auto playerFactionAddr = PVOID(content + factionOffset);
        const auto playerNumber1 = PVOID(content + numberOffset1);
        const auto playerNumber2 = PVOID(content + numberOffset2);
        const auto playerFormation = PCHAR(content + formationOffset);

        if (!conf->callsign.disableRandomisedFormations)
        {
            *(int*)(num1RewriteBytes.data() + 1) = ci.formationNumber1;
            MemUtils::WriteProcMem(playerNumber1, num1RewriteBytes.data(), num1RewriteBytes.size());
            MemUtils::WriteProcMem(playerNumber2, &ci.formationNumber2, 1);
            DWORD _;
            VirtualProtect(playerFormation, 2, PAGE_READWRITE, &_);
            std::sprintf(playerFormation, "%02d", ci.formationTag);
        }

        if (!conf->callsign.disableUsingAffiliationForCallsign)
        {
            if (auto repGroupNick = Hk::IniUtils::i()->GetFromPlayerFile(client, L"rep_group");
                repGroupNick.has_value() && repGroupNick.value().length() - 4 <= 6)
            {
                auto val = StringUtils::wstos(repGroupNick.value());
                MemUtils::WriteProcMem(playerFactionAddr, val.erase(val.size() - 4).c_str(), val.size());
            }
            else
            {
                MemUtils::WriteProcMem(playerFactionAddr, "player", 6);
            }
        }
    }

    func->UnDetour();
    const int res = func->GetOriginalFunc()(fromShipId, toShipId, voiceId, costume, infocardId, a5, a6, infocardId2, a8, a9);
    func->Detour(SendComm);
    return res;
}

void DetourSendComm() { func->Detour(SendComm); }

void UnDetourSendComm() { func->UnDetour(); }
