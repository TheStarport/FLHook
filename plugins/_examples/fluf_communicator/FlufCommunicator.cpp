#include "PCH.hpp"

#include "FlufCommunicator.hpp"
#include <rfl/msgpack.hpp>

using namespace Plugins;

struct KillMessage
{
    std::string textMessage;
};

void FlufCommunicator::OnShipDestroy(Ship* ship, DamageList* dmgList, ShipId killerId) {

    uint flufHeader = 0xf10f;
    static constexpr std::array<char, 4> killMessageHeader{'k', 'i', 'l', 'l'};
    KillMessage km;
    km.textMessage = std::format("They Killed Kenny, those bastards! {} {}", ((CShip*)ship->cobj)->id, killerId.GetId().Unwrap());
    auto msgPack = rfl::msgpack::write(km);

    const size_t size = msgPack.size() + killMessageHeader.size() + sizeof(flufHeader);
    std::vector<char> data(size);

    memcpy_s(data.data(), data.size(), &flufHeader, sizeof(flufHeader));
    memcpy_s(data.data() + sizeof(flufHeader), data.size(), killMessageHeader.data(), killMessageHeader.size());
    memcpy_s(data.data() + msgPack.size() + sizeof(flufHeader), data.size() - msgPack.size(), msgPack.data(), msgPack.size());

    InternalApi::FMsgSendChat(ClientId(1), data.data(), data.size());

}
FlufCommunicator::FlufCommunicator(const PluginInfo& info) : Plugin(info) {}

using namespace Plugins;

DefaultDllMain();

const PluginInfo Info(L"FLUF Communicator", L"fluf_communicator", PluginMajorVersion::V05, PluginMinorVersion::V00);
SetupPlugin(FlufCommunicator, Info);
