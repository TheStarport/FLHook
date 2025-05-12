#pragma once

#include <sol/sol.hpp>

class DLL LuaHelper
{
        LuaHelper() = delete;

        static void LuaPrint(lua_State* lua, const std::wstring& text, LogLevel level);

    public:
        /**
         * @brief Populate a lua state with common user types and global variables to enable calls to FLHook code\n
         * It will by default populate all ID types (other than AccountId), as well as SpaceObjectBuilder, Action<>, and Vector\n
         * It will also provide a logging interface, with the following functions:
         * - log.info
         * - log.warn
         * - log.error
         * - log.trace
         * - log.debug
         * @param lua An already initialised sol state (NOT NULL)
         */
        static void InitialiseDefaultLuaState(sol::state* lua);
};
