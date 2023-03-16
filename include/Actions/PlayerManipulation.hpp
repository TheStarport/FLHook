#pragma once

class DLL PlayerManipulation
{
	static void SetAdmin(const std::variant<uint, std::wstring>& player, const std::wstring& rights);
	static void DelAdmin(const std::variant<uint, std::wstring>& player);
	static void SetReservedSlot(const std::variant<uint, std::wstring>& player, int reservedSlot);
	static void SaveChar(const std::variant<uint, std::wstring>& player);
};