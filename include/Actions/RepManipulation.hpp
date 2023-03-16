#pragma once

class DLL RepManipulation
{
	static void GetRep(const std::variant<uint, std::wstring>& player, const std::wstring& repGroup);
	static void ResetRep(const std::variant<uint, std::wstring>& player);
	static void SetRep(const std::variant<uint, std::wstring>& player, const std::wstring& repGroup, float value);
};