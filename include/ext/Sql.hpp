#pragma once

#pragma warning(push)
#pragma warning(disable : 4067)
#include <SQLiteCpp/SQLiteCpp.h>
#pragma warning(pop)

#pragma comment(lib, "sqlite3.lib")
#pragma comment(lib, "SQLiteCpp.lib")

namespace SqlHelpers
{
	inline SQLite::Database Create(const std::string_view& path)
	{
		char dataPath[MAX_PATH];
		GetUserDataPath(dataPath);

		const auto dir = std::format("{}\\{}", dataPath, path);

		return {dir, SQLite::OPEN_CREATE | SQLite::OPEN_READWRITE};
	};
}; // namespace SqlHelpers