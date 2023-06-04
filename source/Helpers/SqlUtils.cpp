#include "PCH.hpp"
#include "Helpers/SqlUitls.hpp"

struct Test
{
	int id;
	std::wstring name;
};

void SqlUtils::MakeStorage()
{
	auto storage =
	    sqlite_orm::make_storage("test.db", sqlite_orm::make_table("users", sqlite_orm::make_column("name", &Test::name), sqlite_orm::make_column("id", &Test::id)));
	
}