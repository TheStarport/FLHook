#include <PCH.hpp>

#include "API/Types/Id.hpp"

ShipId Id::AsShip() { return ShipId(value); }
ObjectId Id::AsObject() { return ObjectId(value); }
BaseId Id::AsBase() { return BaseId(value); }
SystemId Id::AsSystem() { return SystemId(value); }

Id::Id(const std::wstring_view nickName) : value(CreateID(StringUtils::wstos(nickName).c_str())) {}
Id::Id(const std::string_view nickName) : value(CreateID(nickName.data())) {}
