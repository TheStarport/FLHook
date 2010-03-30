enum PLUGIN_RETURNCODE
{
	DEFAULT_RETURNCODE = 0,
	SKIPPLUGINS = 1,
	SKIPPLUGINS_NOFUNCTIONCALL = 2,
	NOFUNCTIONCALL = 3,
};

enum PLUGIN_MESSAGE;

__declspec(dllexport) void Plugin_Communication(PLUGIN_MESSAGE msgtype, void* msg);