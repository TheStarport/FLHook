#pragma once

class DLL ShipManipulation
{
	static void Beam(const std::variant<uint, std::wstring>& player, const std::wstring& basename);
	static void Chase(std::wstring adminName, const std::variant<uint, std::wstring>& player);
	static void Pull(std::wstring adminName, const std::variant<uint, std::wstring>& player);
	static void Move(std::wstring adminName, float x, float y, float z);
	static void Kill(const std::variant<uint, std::wstring>& player);
};