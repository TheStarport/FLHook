#include "Main.h"

namespace Plugins::Mark
{
	char MarkObject(ClientId client, uint iObject)
	{
		if (!iObject)
			return 1;

		uint iSystemId, iObjectSystemId;
		pub::Player::GetSystem(client, iSystemId);
		pub::SpaceObj::GetSystem(iObject, iObjectSystemId);
		if (iSystemId == iObjectSystemId)
		{
			for (uint i = 0; i < global->Mark[client].MarkedObjects.size(); i++)
			{
				if (global->Mark[client].MarkedObjects[i] == iObject)
					return 3; // already marked
			}
		}
		else
		{
			for (uint j = 0; j < global->Mark[client].DelayedSystemMarkedObjects.size(); j++)
			{
				if (global->Mark[client].DelayedSystemMarkedObjects[j] == iObject)
					return 3; // already marked
			}
			global->Mark[client].DelayedSystemMarkedObjects.push_back(iObject);
			pub::Audio::PlaySoundEffect(client,
			    2460046221); // CreateID("ui_select_add")
			return 0;
		}

		pub::Player::MarkObj(client, iObject, 1);
		for (uint i = 0; i < global->Mark[client].AutoMarkedObjects.size(); i++) // remove from automarked vector
		{
			if (global->Mark[client].AutoMarkedObjects[i] == iObject)
			{
				if (i != global->Mark[client].AutoMarkedObjects.size() - 1)
				{
					global->Mark[client].AutoMarkedObjects[i] = global->Mark[client].AutoMarkedObjects[global->Mark[client].AutoMarkedObjects.size() - 1];
				}
				global->Mark[client].AutoMarkedObjects.pop_back();
				break;
			}
		}
		global->Mark[client].MarkedObjects.push_back(iObject);
		pub::Audio::PlaySoundEffect(client,
		    2460046221); // CreateID("ui_select_add")
		return 0;
	}

	char UnMarkObject(ClientId client, uint iObject)
	{
		if (!iObject)
			return 1;

		for (uint i = 0; i < global->Mark[client].MarkedObjects.size(); i++)
		{
			if (global->Mark[client].MarkedObjects[i] == iObject)
			{
				if (i != global->Mark[client].MarkedObjects.size() - 1)
				{
					global->Mark[client].MarkedObjects[i] = global->Mark[client].MarkedObjects[global->Mark[client].MarkedObjects.size() - 1];
				}
				global->Mark[client].MarkedObjects.pop_back();
				pub::Player::MarkObj(client, iObject, 0);
				pub::Audio::PlaySoundEffect(client,
				    2939827141); // CreateID("ui_select_remove")
				return 0;
			}
		}

		for (uint j = 0; j < global->Mark[client].AutoMarkedObjects.size(); j++)
		{
			if (global->Mark[client].AutoMarkedObjects[j] == iObject)
			{
				if (j != global->Mark[client].AutoMarkedObjects.size() - 1)
				{
					global->Mark[client].AutoMarkedObjects[j] = global->Mark[client].AutoMarkedObjects[global->Mark[client].AutoMarkedObjects.size() - 1];
				}
				global->Mark[client].AutoMarkedObjects.pop_back();
				pub::Player::MarkObj(client, iObject, 0);
				pub::Audio::PlaySoundEffect(client,
				    2939827141); // CreateID("ui_select_remove")
				return 0;
			}
		}

		for (uint k = 0; k < global->Mark[client].DelayedSystemMarkedObjects.size(); k++)
		{
			if (global->Mark[client].DelayedSystemMarkedObjects[k] == iObject)
			{
				if (k != global->Mark[client].DelayedSystemMarkedObjects.size() - 1)
				{
					global->Mark[client].DelayedSystemMarkedObjects[k] =
					    global->Mark[client].DelayedSystemMarkedObjects[global->Mark[client].DelayedSystemMarkedObjects.size() - 1];
				}
				global->Mark[client].DelayedSystemMarkedObjects.pop_back();
				pub::Audio::PlaySoundEffect(client,
				    2939827141); // CreateID("ui_select_remove")
				return 0;
			}
		}
		return 2;
	}

	void UnMarkAllObjects(ClientId client)
	{
		for (uint i = 0; i < global->Mark[client].MarkedObjects.size(); i++)
		{
			pub::Player::MarkObj(client, (global->Mark[client].MarkedObjects[i]), 0);
		}
		global->Mark[client].MarkedObjects.clear();
		for (uint i = 0; i < global->Mark[client].AutoMarkedObjects.size(); i++)
		{
			pub::Player::MarkObj(client, (global->Mark[client].AutoMarkedObjects[i]), 0);
		}
		global->Mark[client].AutoMarkedObjects.clear();
		global->Mark[client].DelayedSystemMarkedObjects.clear();
		pub::Audio::PlaySoundEffect(client,
		    2939827141); // CreateID("ui_select_remove")
	}
} // namespace Plugins::Mark