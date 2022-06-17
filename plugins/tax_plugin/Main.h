#pragma once

// Included
#include <FLHook.h>
#include <plugin.h>

namespace Plugins::Tax
{
	//! Macros
	#define PRINT_DISABLED() PrintUserCmdText(iClientID, L"Command disabled");

	//! Structs
	struct TAX
	{
		uint uiTargetID;
		uint uiInitiatorID;
		std::wstring wscTarget;
		std::wstring wscInitiator;
		int iCash;
		bool f1;
	};

	//! Configurable fields for this plugin
	struct Config final : Reflectable
	{
		std::string File() override { return "flhook_plugins/tax.json"; }

		// Reflectable fields
		bool EnableTax = true;		
		std::vector<std::string> ExcludedSystems = {};
		int MinPlayTimeSec = 0;
		int iMaxTax = 300;

	};
	
	//! Global data for this plugin
	struct Global final
	{
		std::unique_ptr<Config> config = nullptr;
		ReturnCode returncode = ReturnCode::Default;
		std::list<TAX> lsttax;
		std::vector<uint> ExcludedSystemsIds;
	};

	// Tools
	uint _GetTargetID(uint iClientID)
	{
		uint iShip;
		pub::Player::GetShip(iClientID, iShip);
		if (!iShip)
			return 0;
		uint iTarget;
		pub::SpaceObj::GetTarget(iShip, iTarget);
		if (!iTarget)
			return 0;
		uint iTargetClientID = HkGetClientIDByShip(iTarget);
		if (!iTargetClientID)
			return 0;
		return iTargetClientID;
	}

	void PrintUserCmd2(uint iClientID, std::wstring wscText, ...)
	{
		wchar_t wszBuf[1024 * 8] = L"";
		va_list marker;
		va_start(marker, wscText);
		_vsnwprintf(wszBuf, sizeof(wszBuf) - 1, wscText.c_str(), marker);
		std::wstring wstyle = L"0x00FF0090";
		// replace last 2 chars with "10"
		wstyle[8] = 49; // "1"
		wstyle[9] = 48; // "0"
		std::wstring wscXML = L"<TRA data=\"" + wstyle + L"\" mask=\"-1\"/><TEXT>" + XMLText(wszBuf) + L"</TEXT>";
		HkFMsg(iClientID, wscXML);
	}

}

