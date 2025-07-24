#pragma once

#include <Core/FlufInfocardHelper.hpp>
#include <API/FLHook/InfocardManager.hpp>

class FlufInfocardHelper
{
        uint InfocardRangeOffset;

    public:
        explicit FlufInfocardHelper() { InfocardRangeOffset = FLHook::GetInfocardManager()->ReturnPluginInfocardRange(); }

        uint GetGlobalInfocardIds(ushort localIds) const { return InfocardRangeOffset + localIds; }

        void OverrideFlufInfocard(uint ids, const std::wstring& override, bool isName, ClientId client = {}) const
        {
            FLHook::GetInfocardManager()->OverrideInfocard(InfocardRangeOffset + ids, override, isName, client);
        }
};
