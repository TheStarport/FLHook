#include "Mark.h"

namespace Plugins::Mark
{
	void TimerSpaceObjMark()
	{
		try
		{
			if (global->config->AutoMarkRadiusInM <= 0.0f) // automarking disabled
				return;

			PlayerData* playerData = nullptr;
			while ((playerData = Players.traverse_active(playerData)))
			{
				uint client = playerData->onlineId;
				auto ship = Hk::Player::GetShip(client);
				if (ship.has_error() || global->Mark[client].AutoMarkRadius <= 0.0f) // docked or does not want any marking
					continue;

				auto [clientPosition, _] = Hk::Solar::GetLocation(ship.value(), IdType::Ship).value();

				for (uint i = 0; i < global->Mark[client].AutoMarkedObjects.size(); i++)
				{
					auto [targetPosition, _] = Hk::Solar::GetLocation(global->Mark[client].AutoMarkedObjects[i], IdType::Solar).value();
					if (Hk::Math::Distance3D(targetPosition, clientPosition) > global->Mark[client].AutoMarkRadius)
					{
						Hk::Player::MarkObj(client, global->Mark[client].AutoMarkedObjects[i], 0);
						global->Mark[client].DelayedAutoMarkedObjects.push_back(global->Mark[client].AutoMarkedObjects[i]);
						if (i != global->Mark[client].AutoMarkedObjects.size() - 1)
						{
							global->Mark[client].AutoMarkedObjects[i] =
							    global->Mark[client].AutoMarkedObjects[global->Mark[client].AutoMarkedObjects.size() - 1];
							i--;
						}
						global->Mark[client].AutoMarkedObjects.pop_back();
					}
				}

				for (uint i = 0; i < global->Mark[client].DelayedAutoMarkedObjects.size(); i++)
				{
					if (pub::SpaceObj::ExistsAndAlive(global->Mark[client].DelayedAutoMarkedObjects[i]))
					{
						if (i != global->Mark[client].DelayedAutoMarkedObjects.size() - 1)
						{
							global->Mark[client].DelayedAutoMarkedObjects[i] =
							    global->Mark[client].DelayedAutoMarkedObjects[global->Mark[client].DelayedAutoMarkedObjects.size() - 1];
							i--;
						}
						global->Mark[client].DelayedAutoMarkedObjects.pop_back();
						continue;
					}
					auto [targetPosition, _] = Hk::Solar::GetLocation(global->Mark[client].DelayedAutoMarkedObjects[i], IdType::Solar).value();
					if (!(Hk::Math::Distance3D(targetPosition, clientPosition) > global->Mark[client].AutoMarkRadius))
					{
						Hk::Player::MarkObj(client, global->Mark[client].DelayedAutoMarkedObjects[i], 1);
						global->Mark[client].AutoMarkedObjects.push_back(global->Mark[client].DelayedAutoMarkedObjects[i]);
						if (i != global->Mark[client].DelayedAutoMarkedObjects.size() - 1)
						{
							global->Mark[client].DelayedAutoMarkedObjects[i] =
							    global->Mark[client].DelayedAutoMarkedObjects[global->Mark[client].DelayedAutoMarkedObjects.size() - 1];
							i--;
						}
						global->Mark[client].DelayedAutoMarkedObjects.pop_back();
					}
				}
			}
		}
		catch (...)
		{
		}
	}
	void TimerMarkDelay()
	{
		if (!global->DelayedMarks.size())
			return;

		const mstime tmTimeNow = Hk::Time::GetUnixMiliseconds();
		for (auto mark = global->DelayedMarks.begin(); mark != global->DelayedMarks.end();)
		{
			if (tmTimeNow - mark->time > 50)
			{
				auto [itemPosition, _] = Hk::Solar::GetLocation(mark->objId, IdType::Solar).value();

				SystemId itemSystem = Hk::Solar::GetSystemBySpaceId(mark->objId).value();
				// for all players
				PlayerData* playerData = nullptr;
				while ((playerData = Players.traverse_active(playerData)))
				{
					ClientId client = playerData->onlineId;
					if (Players[client].systemId == itemSystem)
					{
						auto [playerPosition, _] = Hk::Solar::GetLocation(Players[client].shipId, IdType::Ship).value();
						if (Hk::Math::Distance3D(playerPosition, itemPosition) <= LOOT_UNSEEN_RADIUS)
						{
							MarkObject(client, mark->objId);
						}
					}
				}
				mark = global->DelayedMarks.erase(mark);
			}
			else
			{
				mark++;
			}
		}
	}

} // namespace Plugins::Mark
