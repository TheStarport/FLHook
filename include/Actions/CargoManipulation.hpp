#pragma once

class DLL CargoManipulation
{
	static void RemoveCargo(const std::variant<uint, std::wstring_view>& player, ushort cargoId, uint count);
	static void AddCargo(const std::variant<uint, std::wstring_view>& player, const std::wstring& good, uint count, bool isMissionItem = false);
};