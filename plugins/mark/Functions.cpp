#include "Mark.h"

namespace Plugins::Mark
{
	const uint UI_SELECT_ADD_SOUND = 2460046221;
	const uint UI_SELECT_REMOVE_SOUND = 2939827141;

	char MarkObject(ClientId client, uint objectId)
	{
		if (!objectId)
			return 1;

		SystemId systemId = Hk::Player::GetSystem(client).value();
		SystemId objectSystemId = Hk::Solar::GetSystemBySpaceId(objectId).value();
		if (systemId == objectSystemId)
		{
			for (uint i = 0; i < global->Mark[client].MarkedObjects.size(); i++)
			{
				if (global->Mark[client].MarkedObjects[i] == objectId)
					return 3; // already marked
			}
		}
		else
		{
			for (uint j = 0; j < global->Mark[client].DelayedSystemMarkedObjects.size(); j++)
			{
				if (global->Mark[client].DelayedSystemMarkedObjects[j] == objectId)
					return 3; // already markediObject
			}
			global->Mark[client].DelayedSystemMarkedObjects.push_back(objectId);
			Hk::Client::PlaySoundEffect(client, UI_SELECT_ADD_SOUND);
			return 0;
		}

		Hk::Player::MarkObj(client, objectId, 1);
		for (uint i = 0; i < global->Mark[client].AutoMarkedObjects.size(); i++) // remove from automarked vector
		{
			if (global->Mark[client].AutoMarkedObjects[i] == objectId)
			{
				if (i != global->Mark[client].AutoMarkedObjects.size() - 1)
				{
					global->Mark[client].AutoMarkedObjects[i] = global->Mark[client].AutoMarkedObjects[global->Mark[client].AutoMarkedObjects.size() - 1];
				}
				global->Mark[client].AutoMarkedObjects.pop_back();
				break;
			}
		}
		global->Mark[client].MarkedObjects.push_back(objectId);
		Hk::Client::PlaySoundEffect(client, UI_SELECT_ADD_SOUND);
		return 0;
	}

	char UnMarkObject(ClientId client, uint objectId)
	{
		if (!objectId)
			return 1;

		for (uint i = 0; i < global->Mark[client].MarkedObjects.size(); i++)
		{
			if (global->Mark[client].MarkedObjects[i] == objectId)
			{
				if (i != global->Mark[client].MarkedObjects.size() - 1)
				{
					global->Mark[client].MarkedObjects[i] = global->Mark[client].MarkedObjects[global->Mark[client].MarkedObjects.size() - 1];
				}
				global->Mark[client].MarkedObjects.pop_back();
				Hk::Player::MarkObj(client, objectId, 0);
				Hk::Client::PlaySoundEffect(client, UI_SELECT_REMOVE_SOUND);
				return 0;
			}
		}

		for (uint j = 0; j < global->Mark[client].AutoMarkedObjects.size(); j++)
		{
			if (global->Mark[client].AutoMarkedObjects[j] == objectId)
			{
				if (j != global->Mark[client].AutoMarkedObjects.size() - 1)
				{
					global->Mark[client].AutoMarkedObjects[j] = global->Mark[client].AutoMarkedObjects[global->Mark[client].AutoMarkedObjects.size() - 1];
				}
				global->Mark[client].AutoMarkedObjects.pop_back();
				Hk::Player::MarkObj(client, objectId, 0);
				Hk::Client::PlaySoundEffect(client, UI_SELECT_REMOVE_SOUND);
				return 0;
			}
		}

		for (uint k = 0; k < global->Mark[client].DelayedSystemMarkedObjects.size(); k++)
		{
			if (global->Mark[client].DelayedSystemMarkedObjects[k] == objectId)
			{
				if (k != global->Mark[client].DelayedSystemMarkedObjects.size() - 1)
				{
					global->Mark[client].DelayedSystemMarkedObjects[k] =
					    global->Mark[client].DelayedSystemMarkedObjects[global->Mark[client].DelayedSystemMarkedObjects.size() - 1];
				}
				global->Mark[client].DelayedSystemMarkedObjects.pop_back();
				Hk::Client::PlaySoundEffect(client, UI_SELECT_REMOVE_SOUND);
				return 0;
			}
		}
		return 2;
	}

	void UnMarkAllObjects(ClientId client)
	{
		for (uint i = 0; i < global->Mark[client].MarkedObjects.size(); i++)
		{
			Hk::Player::MarkObj(client, (global->Mark[client].MarkedObjects[i]), 0);
		}
		global->Mark[client].MarkedObjects.clear();
		for (uint i = 0; i < global->Mark[client].AutoMarkedObjects.size(); i++)
		{
			Hk::Player::MarkObj(client, (global->Mark[client].AutoMarkedObjects[i]), 0);
		}
		global->Mark[client].AutoMarkedObjects.clear();
		global->Mark[client].DelayedSystemMarkedObjects.clear();
		Hk::Client::PlaySoundEffect(client, UI_SELECT_REMOVE_SOUND);
	}
} // namespace Plugins::Mark