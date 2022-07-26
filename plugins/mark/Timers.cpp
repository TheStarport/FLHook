#include "Main.h"

namespace Plugins::Mark
{
	void HkTimerSpaceObjMark()
	{
		try
		{
			if (global->config->AutoMarkRadiusInM <= 0.0f) // automarking disabled
				return;

			struct PlayerData* pPD = 0;
			while (pPD = Players.traverse_active(pPD))
			{
				uint iShip, iClientID = HkGetClientIdFromPD(pPD);
				pub::Player::GetShip(iClientID, iShip);
				if (!iShip || global->Mark[iClientID].AutoMarkRadius <= 0.0f) // docked or does not want any marking
					continue;
				uint iSystem;
				pub::Player::GetSystem(iClientID, iSystem);
				Vector VClientPos;
				Matrix MClientOri;
				pub::SpaceObj::GetLocation(iShip, VClientPos, MClientOri);

				for (uint i = 0; i < global->Mark[iClientID].AutoMarkedObjects.size(); i++)
				{
					Vector VTargetPos;
					Matrix MTargetOri;
					pub::SpaceObj::GetLocation(global->Mark[iClientID].AutoMarkedObjects[i], VTargetPos, MTargetOri);
					if (HkDistance3D(VTargetPos, VClientPos) > global->Mark[iClientID].AutoMarkRadius)
					{
						pub::Player::MarkObj(iClientID, global->Mark[iClientID].AutoMarkedObjects[i], 0);
						global->Mark[iClientID].DelayedAutoMarkedObjects.push_back(global->Mark[iClientID].AutoMarkedObjects[i]);
						if (i != global->Mark[iClientID].AutoMarkedObjects.size() - 1)
						{
							global->Mark[iClientID].AutoMarkedObjects[i] =
							    global->Mark[iClientID].AutoMarkedObjects[global->Mark[iClientID].AutoMarkedObjects.size() - 1];
							i--;
						}
						global->Mark[iClientID].AutoMarkedObjects.pop_back();
					}
				}

				for (uint i = 0; i < global->Mark[iClientID].DelayedAutoMarkedObjects.size(); i++)
				{
					if (pub::SpaceObj::ExistsAndAlive(global->Mark[iClientID].DelayedAutoMarkedObjects[i]))
					{
						if (i != global->Mark[iClientID].DelayedAutoMarkedObjects.size() - 1)
						{
							global->Mark[iClientID].DelayedAutoMarkedObjects[i] =
							    global->Mark[iClientID].DelayedAutoMarkedObjects[global->Mark[iClientID].DelayedAutoMarkedObjects.size() - 1];
							i--;
						}
						global->Mark[iClientID].DelayedAutoMarkedObjects.pop_back();
						continue;
					}
					Vector VTargetPos;
					Matrix MTargetOri;
					pub::SpaceObj::GetLocation(global->Mark[iClientID].DelayedAutoMarkedObjects[i], VTargetPos, MTargetOri);
					if (!(HkDistance3D(VTargetPos, VClientPos) > global->Mark[iClientID].AutoMarkRadius))
					{
						pub::Player::MarkObj(iClientID, global->Mark[iClientID].DelayedAutoMarkedObjects[i], 1);
						global->Mark[iClientID].AutoMarkedObjects.push_back(global->Mark[iClientID].DelayedAutoMarkedObjects[i]);
						if (i != global->Mark[iClientID].DelayedAutoMarkedObjects.size() - 1)
						{
							global->Mark[iClientID].DelayedAutoMarkedObjects[i] =
							    global->Mark[iClientID].DelayedAutoMarkedObjects[global->Mark[iClientID].DelayedAutoMarkedObjects.size() - 1];
							i--;
						}
						global->Mark[iClientID].DelayedAutoMarkedObjects.pop_back();
					}
				}
			}
		}
		catch (...)
		{
		}
	}
	void HkTimerMarkDelay()
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
				uint iItemSystem;
				pub::SpaceObj::GetSystem(mark->iObj, iItemSystem);
				// for all players
				struct PlayerData* pPD = 0;
				while (pPD = Players.traverse_active(pPD))
				{
					uint iClientID = HkGetClientIdFromPD(pPD);
					if (Players[iClientID].iSystemID == iItemSystem)
					{
						pub::SpaceObj::GetLocation(Players[iClientID].iShipID, vPlayer, mTemp);
						if (HkDistance3D(vPlayer, vItem) <= LOOT_UNSEEN_RADIUS)
						{
							HkMarkObject(iClientID, mark->iObj);
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
