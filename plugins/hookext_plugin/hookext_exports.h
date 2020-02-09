namespace HookExt
{
	IMPORT std::wstring IniGetWS(uint client, const std::string &name);
	IMPORT std::wstring IniGetW(uint client, const std::string &name);
	IMPORT uint IniGetI(uint client, const std::string &name);
	IMPORT bool IniGetB(uint client, const std::string &name);
	IMPORT float IniGetF(uint client, const std::string &name);

	IMPORT void IniSetWS(uint client, const std::string &name, const std::wstring &value);
	IMPORT void IniSetS(uint client, const std::string &name, const std::string *value);
	IMPORT void IniSetI(uint client, const std::string &name, uint value);
	IMPORT void IniSetB(uint client, const std::string &name, bool value);
	IMPORT void IniSetF(uint client, const std::string &name, float value);

	IMPORT void IniSetWS(const std::wstring &charname, const std::string &name, const std::wstring &value);
	IMPORT void IniSetS(const std::wstring &charname, const std::string &name, const std::string &value);
	IMPORT void IniSetI(const std::wstring &charname, const std::string &name, uint value);
	IMPORT void IniSetB(const std::wstring &charname, const std::string &name, bool value);
	IMPORT void IniSetF(const std::wstring &charname, const std::string &name, float value);
};