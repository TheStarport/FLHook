#include "Main.h"

namespace Plugins::Mark
{
	void TimerSpaceObjMark()
	{
		try
		{
			if (global->config->AutoMarkRadiusInM <= 0.0f) // automarking disabled
				return;

			struct PlayerData* playerData = 0;
			while (playerData = Players.traverse_active(playerData))
			{
				uint ship, client = playerData->iOnlineId;
				pub::Player::GetShip(client, ship);
				if (!ship || global->Mark[client].AutoMarkRadius <= 0.0f) // docked or does not want any marking
					continue;
				SystemId iSystem = Hk::Player::GetSystem(client).value();
				Vector VClientPos;
				Matrix MClientOri;
				pub::SpaceObj::GetLocation(ship, VClientPos, MClientOri);

				for (uint i = 0; i < global->Mark[client].AutoMarkedObjects.size(); i++)
				{
					Vector VTargetPos;
					Matrix MTargetOri;
					pub::SpaceObj::GetLocation(global->Mark[client].AutoMarkedObjects[i], VTargetPos, MTargetOri);
					if (Hk::Math::Distance3D(VTargetPos, VClientPos) > global->Mark[client].AutoMarkRadius)
					{
						pub::Player::MarkObj(client, global->Mark[client].AutoMarkedObjects[i], 0);
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
					Vector VTargetPos;
					Matrix MTargetOri;
					pub::SpaceObj::GetLocation(global->Mark[client].DelayedAutoMarkedObjects[i], VTargetPos, MTargetOri);
					if (!(Hk::Math::Distance3D(VTargetPos, VClientPos) > global->Mark[client].AutoMarkRadius))
					{
						pub::Player::MarkObj(client, global->Mark[client].DelayedAutoMarkedObjects[i], 1);
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

		mstime tmTimeNow = timeInMS();
		for (std::list<DELAY_MARK>::iterator mark = global->DelayedMarks.begin(); mark != global->DelayedMarks.end();)
		{
			if (tmTimeNow - mark->time > 50)
			{
				Matrix mTemp;
				Vector vItem, vPlayer;
				pub::SpaceObj::GetLocation(mark->iObj, vItem, mTemp);
				SystemId iItemSystem = Hk::Solar::GetSystemBySpaceId(mark->iObj).value();
				// for all players
				struct PlayerData* playerData = 0;
				while (playerData = Players.traverse_active(playerData))
				{
					ClientId client = playerData->iOnlineId;
					if (Players[client].systemId == iItemSystem)
					{
						pub::SpaceObj::GetLocation(Players[client].shipId, vPlayer, mTemp);
						if (Hk::Math::Distance3D(vPlayer, vItem) <= LOOT_UNSEEN_RADIUS)
						{
							MarkObject(client, mark->iObj);
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

}
