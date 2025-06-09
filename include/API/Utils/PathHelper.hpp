#pragma once

#include <FLCore/Server/Paths.hpp>

class PathHelper
{
        PathHelper() = delete;

    public:
        static void ClearWaypoints(ClientId);
        static void CreateObjectWaypoint(ClientId, const ObjectId&);
        static void CreateClearableWaypoints(ClientId, const std::vector<PathEntry>& path);
};
