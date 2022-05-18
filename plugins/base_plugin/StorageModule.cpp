#include "Main.h"

StorageModule::StorageModule(PlayerBase* the_base) : Module(TYPE_STORAGE), base(the_base)
{
}

StorageModule::~StorageModule()
{
}

std::wstring StorageModule::GetInfo(bool xml)
{
	return L"Cargo Storage";
}

void StorageModule::LoadState(INI_Reader& ini)
{
	while (ini.read_value())
	{
	}
}

void StorageModule::SaveState(FILE* file)
{
	fprintf(file, "[StorageModule]\n");
}
