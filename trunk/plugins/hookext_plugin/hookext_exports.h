namespace HookExt
{
	IMPORT wstring IniGetWS(uint client, const string &name);
	IMPORT wstring IniGetW(uint client, const string &name);
	IMPORT uint IniGetI(uint client, const string &name);
	IMPORT bool IniGetB(uint client, const string &name);
	IMPORT float IniGetF(uint client, const string &name);

	IMPORT void IniSetWS(uint client, const string &name, const wstring &value);
	IMPORT void IniSetS(uint client, const string &name, const string *value);
	IMPORT void IniSetI(uint client, const string &name, uint value);
	IMPORT void IniSetB(uint client, const string &name, bool value);
	IMPORT void IniSetF(uint client, const string &name, float value);

	IMPORT void IniSetWS(const wstring &charname, const string &name, const wstring &value);
	IMPORT void IniSetS(const wstring &charname, const string &name, const string &value);
	IMPORT void IniSetI(const wstring &charname, const string &name, uint value);
	IMPORT void IniSetB(const wstring &charname, const string &name, bool value);
	IMPORT void IniSetF(const wstring &charname, const string &name, float value);
};