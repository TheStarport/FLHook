#pragma once

#include <sw/redis++/redis.h>

class DLL Database : public Singleton<Database>
{
        std::optional<sw::redis::Redis> redis;

    public:
        Database();
};
