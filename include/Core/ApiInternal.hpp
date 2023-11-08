#pragma once

#ifdef FLHOOK

namespace HkApi
{
    ClientId ExtractClientID(const std::variant<uint, std::wstring_view>& player);

}
#endif
