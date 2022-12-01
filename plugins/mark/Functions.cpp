#include "Main.h"

namespace Plugins::Mark
{
	char MarkObject(uint iClientID, uint iObject)
	{
		if (!iObject)
			return 1;

		uint iClientIDMark = GetClientIDByShip(iObject);

		uint iSystemID, iObjectSystemID;
		pub::Player::GetSystem(iClientID, iSystemID);
		pub::SpaceObj::GetSystem(iObject, iObjectSystemID);
		if (iSystemID == iObjectSystemID)
		{
			for (uint i = 0; i < global->Mark[iClientID].MarkedObjects.size(); i++)
			{
				if (global->Mark[iClientID].MarkedObjects[i] == iObject)
					return 3; // already marked
			}
		}
		else
		{
			for (uint j = 0; j < global->Mark[iClientID].DelayedSystemMarkedObjects.size(); j++)
			{
				if (global->Mark[iClientID].DelayedSystemMarkedObjects[j] == iObject)
					return 3; // already marked
			}
			global->Mark[iClientID].DelayedSystemMarkedObjects.push_back(iObject);
			pub::Audio::PlaySoundEffect(iClientID,
			    2460046221); // CreateID("ui_select_add")
			return 0;
		}

		pub::Player::MarkObj(iClientID, iObject, 1);
		for (uint i = 0; i < global->Mark[iClientID].AutoMarkedObjects.size(); i++) // remove from automarked vector
		{
			if (global->Mark[iClientID].AutoMarkedObjects[i] == iObject)
			{
				if (i != global->Mark[iClientID].AutoMarkedObjects.size() - 1)
				{
					global->Mark[iClientID].AutoMarkedObjects[i] = global->Mark[iClientID].AutoMarkedObjects[global->Mark[iClientID].AutoMarkedObjects.size() - 1];
				}
				global->Mark[iClientID].AutoMarkedObjects.pop_back();
				break;
			}
		}
		global->Mark[iClientID].MarkedObjects.push_back(iObject);
		pub::Audio::PlaySoundEffect(iClientID,
		    2460046221); // CreateID("ui_select_add")
		return 0;
	}

	char UnMarkObject(uint iClientID, uint iObject)
	{
		if (!iObject)
			return 1;

		for (uint i = 0; i < global->Mark[iClientID].MarkedObjects.size(); i++)
		{
			if (global->Mark[iClientID].MarkedObjects[i] == iObject)
			{
				if (i != global->Mark[iClientID].MarkedObjects.size() - 1)
				{
					global->Mark[iClientID].MarkedObjects[i] = global->Mark[iClientID].MarkedObjects[global->Mark[iClientID].MarkedObjects.size() - 1];
				}
				global->Mark[iClientID].MarkedObjects.pop_back();
				pub::Player::MarkObj(iClientID, iObject, 0);
				pub::Audio::PlaySoundEffect(iClientID,
				    2939827141); // CreateID("ui_select_remove")
				return 0;
			}
		}

		for (uint j = 0; j < global->Mark[iClientID].AutoMarkedObjects.size(); j++)
		{
			if (global->Mark[iClientID].AutoMarkedObjects[j] == iObject)
			{
				if (j != global->Mark[iClientID].AutoMarkedObjects.size() - 1)
				{
					global->Mark[iClientID].AutoMarkedObjects[j] = global->Mark[iClientID].AutoMarkedObjects[global->Mark[iClientID].AutoMarkedObjects.size() - 1];
				}
				global->Mark[iClientID].AutoMarkedObjects.pop_back();
				pub::Player::MarkObj(iClientID, iObject, 0);
				pub::Audio::PlaySoundEffect(iClientID,
				    2939827141); // CreateID("ui_select_remove")
				return 0;
			}
		}

		for (uint k = 0; k < global->Mark[iClientID].DelayedSystemMarkedObjects.size(); k++)
		{
			if (global->Mark[iClientID].DelayedSystemMarkedObjects[k] == iObject)
			{
				if (k != global->Mark[iClientID].DelayedSystemMarkedObjects.size() - 1)
				{
					global->Mark[iClientID].DelayedSystemMarkedObjects[k] =
					    global->Mark[iClientID].DelayedSystemMarkedObjects[global->Mark[iClientID].DelayedSystemMarkedObjects.size() - 1];
				}
				global->Mark[iClientID].DelayedSystemMarkedObjects.pop_back();
				pub::Audio::PlaySoundEffect(iClientID,
				    2939827141); // CreateID("ui_select_remove")
				return 0;
			}
		}
		return 2;
	}

	void UnMarkAllObjects(uint iClientID)
	{
		for (uint i = 0; i < global->Mark[iClientID].MarkedObjects.size(); i++)
		{
			pub::Player::MarkObj(iClientID, (global->Mark[iClientID].MarkedObjects[i]), 0);
		}
		global->Mark[iClientID].MarkedObjects.clear();
		for (uint i = 0; i < global->Mark[iClientID].AutoMarkedObjects.size(); i++)
		{
			pub::Player::MarkObj(iClientID, (global->Mark[iClientID].AutoMarkedObjects[i]), 0);
		}
		global->Mark[iClientID].AutoMarkedObjects.clear();
		global->Mark[iClientID].DelayedSystemMarkedObjects.clear();
		pub::Audio::PlaySoundEffect(iClientID,
		    2939827141); // CreateID("ui_select_remove")
	}
} // namespace Plugins::Mark