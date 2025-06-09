#include "PCH.hpp"

#include "API/Utils/PathHelper.hpp"

void PathHelper::ClearWaypoints(ClientId client)
{

    RequestPath<0> emptyPath;
    emptyPath.repId = client.GetReputation().Handle().GetValue();
    emptyPath.waypointCount = 0;
    emptyPath.noPathFound = false;

    pub::Player::ReturnBestPath(client.GetValue(), reinterpret_cast<uchar*>(&emptyPath), sizeof(emptyPath));
}

void PathHelper::CreateObjectWaypoint(ClientId client, const ObjectId& object)
{
    RequestPath<1> objectPath;
    objectPath.repId = client.GetReputation().Handle().GetValue();
    objectPath.waypointCount = 1;
    objectPath.noPathFound = false;
    objectPath.pathEntries[0].objId = object.GetId().Handle();

    pub::Player::ReturnBestPath(client.GetValue(), reinterpret_cast<uchar*>(&objectPath), sizeof(objectPath));
}

void PathHelper::CreateClearableWaypoints(ClientId client, const std::vector<PathEntry>& path)
{
    if (path.size() > 50)
    {
        return;
    }

    RequestPath<50> objectPath;
    objectPath.repId = client.GetReputation().Handle().GetValue();
    objectPath.waypointCount = path.size();
    objectPath.noPathFound = false;
    objectPath.funny[0] = true;
    objectPath.funny[1] = true;
    objectPath.funny[2] = true;

    memcpy_s(objectPath.pathEntries, sizeof(PathEntry) * path.size(), path.data(), sizeof(PathEntry) * path.size());

    pub::Player::ReturnBestPath(client.GetValue(), reinterpret_cast<uchar*>(&objectPath), 12 + sizeof(PathEntry) * path.size());
}
