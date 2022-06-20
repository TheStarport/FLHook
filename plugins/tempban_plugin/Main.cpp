#include "Main.h"

namespace Plugins::Tempban
{
	const std::unique_ptr<Global> global = std::make_unique<Global>();

	/**************************************************************************************************************
	Check if TempBans exceeded
	**************************************************************************************************************/

	void HkTimerCheckKick()
	{
		// timed out tempbans get deleted here

		global->returncode = ReturnCode::Default;

		for (auto it = global->TempBans.begin(); it != global->TempBans.end(); ++it)
		{
			if (((*it).banstart + (*it).banduration) < timeInMS())
			{
				global->TempBans.erase(it);
				break; // fix to not overflow the list
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	HK_ERROR __stdcall HkTempBan(const std::wstring& wscCharname, uint _duration)
	{
		const uint iClientID = HkGetClientIdFromCharname(wscCharname);

		mstime duration = 1000 * _duration * 60;
		TEMPBAN_INFO tempban;
		tempban.banstart = timeInMS();
		tempban.banduration = duration;

		CAccount* acc;
		if (iClientID != -1)
			acc = Players.FindAccountFromClientID(iClientID);
		else
		{
			if (!(acc = HkGetAccountByCharname(wscCharname)))
				return HKE_CHAR_DOES_NOT_EXIST;
		}
		std::wstring wscID = HkGetAccountID(acc);

		tempban.wscID = wscID;
		global->TempBans.push_back(tempban);

		return HKE_OK;
	}

	bool HkTempBannedCheck(uint iClientID)
	{
		CAccount* acc;
		acc = Players.FindAccountFromClientID(iClientID);

		std::wstring wscID = HkGetAccountID(acc);

		for (auto& ban : global->TempBans)
		{
			if (ban.wscID == wscID)
				return true;
		}

		return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////


	void __stdcall Login(struct SLoginInfo const& li, unsigned int& iClientID)
	{
		global->returncode = ReturnCode::Default;

		// check for tempban
		if (HkTempBannedCheck(iClientID))
		{
			global->returncode = ReturnCode::SkipAll;
			HkKick(iClientID);
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void CmdTempBan(CCmds* classptr, const std::wstring& wscCharname, uint iDuration)
	{
		// right check
		if (!(classptr->rights & RIGHT_KICKBAN))
		{
			classptr->Print(L"ERR No permission");
			return;
		}

		if (((classptr->hkLastErr = HkTempBan(wscCharname, iDuration)) == HKE_OK)) // hksuccess
			classptr->Print(L"OK");
		else
			classptr->PrintError();
	}

	bool ExecuteCommandString(CCmds* classptr, const std::wstring& wscCmd)
	{
		if (wscCmd == L"tempban")
		{
			global->returncode = ReturnCode::SkipAll; // do not let other plugins kick in
			                                  // since we now handle the command

			CmdTempBan(classptr, classptr->ArgCharname(1), classptr->ArgInt(2));

			return true;
		}

		return false;
	}

	void CmdHelp(CCmds* classptr) { classptr->Print(L"tempban <charname>"); }

	TempBanCommunicator::TempBanCommunicator(std::string plug) : PluginCommunicator(plug) { this->TempBan = HkTempBan; }
} // namespace Plugins::Tempban

using namespace Plugins::Tempban;

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	// clear tempban list
	global->TempBans.clear();

	return true;
}

extern "C" EXPORT void ExportPluginInfo(PluginInfo* pi)
{
	pi->name(TempBanCommunicator::pluginName);
	pi->shortName("tempban");
	pi->mayPause(true);
	pi->mayUnload(true);
	pi->returnCode(&global->returncode);
	pi->versionMajor(PluginMajorVersion::VERSION_04);
	pi->versionMinor(PluginMinorVersion::VERSION_00);
	pi->emplaceHook(HookedCall::FLHook__TimerCheckKick, &HkTimerCheckKick);
	pi->emplaceHook(HookedCall::IServerImpl__Login, &Login, HookStep::After);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Process, &ExecuteCommandString);
	pi->emplaceHook(HookedCall::FLHook__AdminCommand__Help, &CmdHelp);

	// Register IPC
	global->communicator = new TempBanCommunicator(TempBanCommunicator::pluginName);
	PluginCommunicator::ExportPluginCommunicator(global->communicator);
}