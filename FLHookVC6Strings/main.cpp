#include <windows.h>
#include <string>
#include <map>
using namespace std;

extern "C" __declspec(dllexport) wstring* CreateWString(wchar_t *wszStr)
{
	wstring *wscStr = new wstring;
	*wscStr = wszStr;
	return wscStr;
}

extern "C" __declspec(dllexport) void FreeWString(wstring *wscStr)
{
	delete wscStr;
}

extern "C" __declspec(dllexport) string* CreateString(char *szStr)
{
	string *scStr = new string;
	*scStr = szStr;
	return scStr;
}

extern "C" __declspec(dllexport) void FreeString(string *scStr)
{
	delete scStr;
}

extern "C" __declspec(dllexport) const char* GetCString(string *scStr)
{
	return scStr->c_str();
}

extern "C" __declspec(dllexport) const wchar_t* GetWCString(wstring *wscStr)
{
	return wscStr->c_str();
}

extern "C" __declspec(dllexport) void WStringAssign(wstring *wscStr, const wchar_t *wszCStr)
{
	wscStr->assign(wszCStr);
}

extern "C" __declspec(dllexport) void WStringAppend(wstring *wscStr, const wchar_t *wszCStr)
{
	wscStr->append(wszCStr);
}

#define OBJECT_DATA_SIZE	2048
#pragma comment(lib, "../plugins/flhookplugin_sdk/libs/FLCoreCommon.lib")

class __declspec(dllimport) CPlayerAccount
{
public:
	CPlayerAccount(class CPlayerAccount const &);
	CPlayerAccount(void);
	virtual ~CPlayerAccount(void);
	class CPlayerAccount & operator=(class CPlayerAccount const &);
	void GenerateAccount(char const *);
	static bool  GenerateTextKey(char *);
	wstring GetAccountName(void);
	char const * GetAccountNameChar(void);
	wstring GetAccountNameSig(void);
	char const * GetAccountNameSigChar(void);
	wstring GetServerSignature(char const *);
	static bool  GetTextKey(char *);
	bool LoadAccount(void);
	bool LoadAccountFromStrings(char const *, char const *);
	static void  SetAcctIndex(unsigned long);
	static bool  SetTextKey(char *);
	bool VerifyAccount(void);

protected:
	void TossHashes(void);
	static unsigned long  g_dwAccountIndex;

public:
	unsigned char data[OBJECT_DATA_SIZE];
};

extern "C" __declspec(dllexport) wstring* CPlayerAccount_GetServerSignature(CPlayerAccount* cpa, const char* sig)
{
	wstring ws = cpa->GetServerSignature(sig);

	wstring* ws2 = new wstring;
	ws2->assign(ws);

	return ws2;
}
