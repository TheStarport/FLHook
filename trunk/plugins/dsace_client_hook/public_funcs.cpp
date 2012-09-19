/// DSAce for Freelancer by Cannon 19/Sept/10
///
/// Thanks to Adoxa, Wodka, Motah for helping me out with this.

#include "dllmain.h"
#include "equip_funcs.h"

static DWORD set_iColorHostile = 0;
static DWORD set_iColorNeutral = 0;
static DWORD set_iColorFriendly = 0;
static DWORD set_iColorGroup = 0;

static DWORD rgb_to_bgr(DWORD rgb)
{
	DWORD color = 0;
	if (rgb)
	{
		color = 0xFF000000;
		color |= (rgb & 0xFF0000) >> 16;
		color |= (rgb & 0x00FF00);
		color |= (rgb & 0x0000FF) << 16;
	}
	return color;
}

// Allow ship roll in mouse flight ~ Jason Hood
__declspec(naked) void HkCb_Roll(void)
{
	__asm
	{
		xor	eax, eax
		cmp	dword ptr [edx], 0x80000000	// No X?
		jne	roll
		cmp	dword ptr [edx+4], 0x80000000	// No Y?
		jne	roll
		mov	eax, [edx+8]			// Must be Z
	roll:
		mov	ds:[0x4c7f14], eax
		jmp	dword ptr ds:[0x5c646c]
	}
}

// Toggle the state of the hud ~ Jason Hood
static void ToggleHud()
{
	static bool hud_showing = true;

	hud_showing = !hud_showing;

	BOOL val = (hud_showing) ? 1 : 0;
	WriteProcMem((char*)0x679c0c, &val, 1);
	WriteProcMem((char*)0x679c10, &val, 1);
	WriteProcMem((char*)0x679c20, &val, 1);
	WriteProcMem((char*)0x679c40, &val, 1);
}


// Toggle to colour of some hud elements.
static void ToggleHudColor()
{
	static bool alt_color_on = true;
	alt_color_on = !alt_color_on;

	static DWORD iDefColorHostile = 0x00000000;
	static DWORD iDefColorNeutral = 0x00000000; 
	static DWORD iDefColorFriendly = 0x00000000;
	static DWORD iDefColorGroup = 0x00000000;
	if (!iDefColorHostile)
	{
		ReadProcMem((char*)0x679BA8, &iDefColorHostile, 4);
		ReadProcMem((char*)0x679B84, &iDefColorNeutral, 4);
		ReadProcMem((char*)0x679BAC, &iDefColorFriendly, 4);
		ReadProcMem((char*)0x679B88, &iDefColorGroup, 4);
	}

	if (alt_color_on)
	{
		WriteProcMem((char*)0x679BA8, &set_iColorHostile, 4);
		WriteProcMem((char*)0x679B84, &set_iColorNeutral, 4);
		WriteProcMem((char*)0x679BAC, &set_iColorFriendly, 4);
		WriteProcMem((char*)0x679B88, &set_iColorGroup, 4);
	}
	else
	{
		WriteProcMem((char*)0x679BA8, &iDefColorHostile, 4);
		WriteProcMem((char*)0x679B84 , &iDefColorNeutral, 4);
		WriteProcMem((char*)0x679BAC, &iDefColorFriendly, 4);
		WriteProcMem((char*)0x679B88, &iDefColorGroup, 4);
	}
}
typedef byte* (*_GetObjByStr)(char*);
_GetObjByStr GetObjByStr = (_GetObjByStr)0x59DA10;

extern bool bTurretMode;

IObjRW* GetPlayerShip();

float *dword_616840 = (float*)0x616840; // mouse pos x
float *dword_616844 = (float*)0x616844; // mouse pos y
float *dword_610850 = (float*)0x610850;
float *dword_610854 = (float*)0x610854;
byte *byte_673554 = (byte*)0x673554;
byte *byte_67354C = (byte*)0x67354C; // mouse flight on

void SetDir()
{
	float v21 = (*dword_616840 - *dword_610850 * 0.5f) / (*dword_610850 * 0.5f);
	float v19 = *dword_610854 * 0.5f;
	
	//	if ( (!(unsigned __int8)sub_45A470() && byte_67354C || *(float *)&dword_673550 >= (double)flt_612270)
    //        && sub_5792B0(23) )
    //        v9 = (double)(dword_610854 - LODWORD(dword_616844));
    //      else
    float v9 = *dword_616844;

	float v20 = (v9 - v19) / v19;
	
	Vector v;
	v.x = v21;
	v.y = v20;
	v.z = 0;

	IBehaviorManager *m = GetBehaviorManager(GetPlayerShip());
	m->update_current_behavior_direction(v);

	*byte_673554 = 1;
}


bool __stdcall HkCb_HandleKeyCmd(int keycmd, int dunno)
{
	switch (keycmd)
	{
	// Mouse wheel event
	case 0x04:
		// If left control is down then the mouse wheel controls the zoom
		if (GetKeyState(VK_LCONTROL) & 0x8000)
		{
			SetTurretCamZoom((float)*((int*)0x61684C));
			return true;
		}
		return false;
	// Print screen button
	//case 0xcb:
	//	return SendChatToServer(0, 0xFFFF, L"/ss*");
	default:
		break;
	}

	// Don't do any further processing if the chat window is open
	byte* obj = (byte*)GetObjByStr("CTID");
	if (obj && (*(obj+108) & 0x03))
	{
		return false;
	}

	switch (keycmd)
	{
	case 0x27:
		SetTurretCamZoom(1.0f);
		return true;
	case 0x28:
		SetTurretCamZoom(-1.0f);
		return true;

	/* case 0x32:
	// case 0xb7: // turn left
		{
			if (bTurretMode)
			{
				AddLog(L"turn left\n");


				SetDir();

				IObjInspectImpl *ship = (IObjInspectImpl *)GetPlayerShip();
				
				IBehaviorManager *m = GetBehaviorManager(GetPlayerShip());
				Vector v = { -0.4, 0, 0 };
				m->set_user_turning_input_state(false);
				m->update_current_behavior_auto_level(false);
				m->update_current_behavior_direction(v);
				return true;
			}
		}
		break;

	case 0x33:
	//case 0xb8: // turn right
		{
			if (bTurretMode)
			{
				AddLog(L"turn right\n");

				IObjInspectImpl *ship = (IObjInspectImpl *)GetPlayerShip();

				SetDir();

				IBehaviorManager *m = GetBehaviorManager(GetPlayerShip());
				Vector v = { 0.4, 0, 0 };
				m->set_user_turning_input_state(false);
				m->update_current_behavior_auto_level(false);
				m->update_current_behavior_direction(v);

				m->get_user_turning_input_state();
				//m->set_camera_mode(IBehaviorManager::B); 
				return true;
			}
		}
		break;

	case 0x34:
	//case 0xb9: // up
		{
			if (bTurretMode)
			{
				AddLog(L"up\n");

				IObjInspectImpl *ship = (IObjInspectImpl *)GetPlayerShip();
				
				IBehaviorManager *m = GetBehaviorManager(GetPlayerShip());
				Vector v = { 0, 0, 0};
				m->set_user_turning_input_state(false);
				m->update_current_behavior_auto_level(true);
				m->update_current_behavior_direction(v);
				return true;
			}
		}
		break;

	case 0x35:
	//case 0xba: // down
		{
			if (bTurretMode)
			{
				AddLog(L"down\n");

				IObjInspectImpl *ship = (IObjInspectImpl *)GetPlayerShip();
				
				IBehaviorManager *m = GetBehaviorManager(GetPlayerShip());
				Vector v = m->get_ship_up_direction();
				AddLog(L"%f %f %f\n", v.x, v.y, v.z);				
				return true;
			}
		}
		break;
	case 0x36:
		{
			if (bTurretMode)
			{
				IBehaviorManager *m = GetBehaviorManager(GetPlayerShip());
				Vector v = { 0, 0, 0};
				m->update_current_behavior_direction(v);
				return true;
			}
		}
		break; */

	//case 0x79:
	//	SendChatToServer(0, 0xFFFF, L"/dock*");
	//	return false;
	case 0x7b:
		SendChatToServer(0, 0xFFFF, L"/jump*");
		return true;
	case 0x7c:
		ToggleHud();
		return true;
	case 0x7d:	
		ToggleHudColor();
		return true;
	case 0x7e:
		SendChatToServer(0, 0xFFFF, L"/lights*");
		return true;
	case 0x80:
		SendChatToServer(0, 0xFFFF, L"/showinfo");
		return true;
	case 0x81:
		SendChatToServer(0, 0xFFFF, L"/selfdestruct*");
		return true;
	case 0x82:
		SendChatToServer(0, 0xFFFF, L"/shields*");
		return true;
	case 0x83:
		SendChatToServer(0, 0xFFFF, L"/marktarget*");
		return true;
	case 0x84:
		SendChatToServer(0, 0xFFFF, L"/unmarktarget*");
		return true;
	case 0x8e:
		SendChatToServer(0, 0xFFFF, L"/cloak*");
		return true;
	default:
		if (keycmd>0x06 && set_iDebug == 23)
			AddLog(L"Unrecognised key cmd=%08x dunno=%08x\n", keycmd, dunno);
	}
	return false;
}

__declspec(naked) void HkCb_HandleKeyCmdNaked(void)
{
	__asm
	{
		push [esp+8] 
		push [esp+8]
		call HkCb_HandleKeyCmd
		test al, al
		jz   cmd_not_processed
		ret

cmd_not_processed:
		sub esp, 80h
		push ebx
		push 0x00576417
		ret
	}
}

/// List of infocards defined in infocards.cfg in FLDev import format.
static map<uint, wstring> set_mapIDStrings;
static void LoadInfocards()
{
#define STATE_WAITING_FOR_IDS 0
#define STATE_WAITING_FOR_TYPE 1
#define STATE_WAITING_FOR_NAME 2
#define STATE_WAITING_FOR_INFOCARD 3

	FILE *file = fopen("infocards.cfg", "r");
	if (file)
	{
		char buf[0xFFFF];
		uint state = STATE_WAITING_FOR_IDS;
		uint ids = 0;
		string text;
		while (fgets(buf, sizeof(buf), file))
		{
			string st = Trim(buf);
			if (st.length()==0)
				continue;
			if (st[0] == ';')
				continue;

			switch (state)
			{
			case STATE_WAITING_FOR_IDS:
				ids = atoi(buf);
				if (ids > 0)
					state = STATE_WAITING_FOR_TYPE;
				break;
			case STATE_WAITING_FOR_TYPE:
				if (st=="NAME")
					state = STATE_WAITING_FOR_NAME;
				else if (st=="INFOCARD")
					state = STATE_WAITING_FOR_INFOCARD;
				break;
			case STATE_WAITING_FOR_NAME:
			case STATE_WAITING_FOR_INFOCARD:
				set_mapIDStrings[ids] = stows(st);
				state = STATE_WAITING_FOR_IDS;
				break;
			default:
				state = STATE_WAITING_FOR_IDS;
				break;
			}
		}
		fclose(file);
	}
}

/// Return the infocard text or 0 if custom infocard does not exist.
static const wchar_t *GetCustomIDS(uint ids_number)
{
	// Search for server provided infocard
	map<uint, wstring>::const_iterator iter = set_mapServerIDStrings.find(ids_number);
	if (iter!=set_mapServerIDStrings.end())
		return iter->second.c_str();

	// Search for infocard.ini provided infocard
	iter = set_mapIDStrings.find(ids_number);
	if (iter!=set_mapIDStrings.end())
		return iter->second.c_str();

	return 0;
}

/// Override ID strings.
static int GetIDSStringResult;
int __stdcall HkCb_GetIDSString(int rsrc, unsigned int ids_number, wchar_t *buf, unsigned int buf_size)
{
	const wchar_t *text = GetCustomIDS(ids_number);
	if (text)
	{
		//AddLog(L"Overriding infocard: %d [%s]\n", ids_number, text);	
		size_t num_chars = wcslen(text);
		wcsncpy(buf, text, buf_size);
		buf[buf_size - 1] = 0;
		return num_chars;
	}
	return 0;
}

/// Naked function hook for the infoname/infocard function.
__declspec(naked) void HkCb_GetIDSStringNaked()
{
	__asm {
		push [esp+16] 
		push [esp+16]
		push [esp+16]
		push [esp+16]
		call HkCb_GetIDSString
		cmp eax, 0
		jnz done

		mov eax, [esp+8]
		mov ecx, [esp+4]
		push ebx
		push 0x4347E9
done:
		ret
	}
}

typedef int (__cdecl *_GetString)(char *rsrc, uint ids, wchar_t *buf, int buf_chars);
extern _GetString GetString = (_GetString)0x4347E0;

typedef int (__cdecl *_FormatXML)(const wchar_t *buf, int buf_chars, RenderDisplayList &rdl, int iDunno);
extern _FormatXML FormatXML = (_FormatXML)0x57DB50;

/// Return true if we override the default infocard
bool __stdcall HkCb_GetInfocard(unsigned int ids_number, RenderDisplayList &rdl)
{
	const wchar_t *text = GetCustomIDS(ids_number);
	if (text)
	{
		// AddLog(L"Overriding infocard: %d [%s]\n", ids_number, text);	
		XMLReader reader;
		size_t num_chars = wcslen(text);
		if (!reader.read_buffer(rdl, (const char*)text, num_chars * 2))
		{
			FormatXML(text, num_chars, rdl, 1);		
		}
		return true;
	}

	return false;
}

/// Naked function hook for the infocard function.
__declspec(naked) void HkCb_GetInfocardNaked()
{
	__asm {
		push [esp+8]
		push [esp+8]
		call HkCb_GetInfocard
		cmp al, 1
		jnz   infocard_not_found
		retn
infocard_not_found:
		mov edx, ds:[0x67C400]
		push ebx
		push 0x057DA47
		ret
	}
}

__declspec(naked) void HkCb_InitShipNaked()
{
	__asm {
		push eax
		call InitTurretCam
		pop eax
		retn 4
	}
}


__declspec(naked) void HkCb_SetCamNaked()
{
	__asm {
		push [esp + 4]
		call SetCamType
		retn 8
	}
}

void SetBackGroundRunPatch(bool bInWindowedMode)
{
	if (bInWindowedMode)
	{
		BYTE patch[] = { 0xBA, 0x01, 0x00, 0x00, 0x00, 0x90};
		WriteProcMem( (char*)0x5B264C, &patch, 6);
	}
	else
	{
		BYTE patch[] = { 0x83, 0xFF, 0x01, 0x0F, 0x94, 0xC2};
		WriteProcMem( (char*)0x5B264C, &patch, 6);
	}
}

// Install client side hooks and patches
void PublicClientPatchInstall()
{
	HMODULE hFreelancer = GetModuleHandleA(0);
	HMODULE hCommon = GetModuleHandleA("common.dll");

	// Patch to reduce formation catch up speed so that slow multi-cruise ships 
	// cannot catch up to faster multi-cruise ships.
	float fFormationSpeedMult = 1.02f;
	WriteProcMem((char*)hCommon + 0x7637f, &fFormationSpeedMult, 4);

	// Patch to keep FL display running when the window doesn't have focus 
	// when in windowed mode
	if (GetBoolSetting("-w"))
	{
		SetBackGroundRunPatch(true);
	}

	// Patch for no new player messages.
	if (!GetBoolSetting("-newplayer"))
	{
		BYTE  patch1[] = { 0x90, 0xE9 };
		WriteProcMem((char*)0x46AAF8, &patch1, 2);
	}

	// Patch for departing player messages.
	if (!GetBoolSetting("-dptplayer"))
	{
		BYTE  patch1[] = { 0x90, 0xE9 };
		WriteProcMem((char*)0x46AF1E, &patch1, 2);
	}

	// Show turret view/mouse flight text
	if (!GetBoolSetting("-showflighttext"))
	{
		BYTE patch1[] = { 0xC3 };
		WriteProcMem((char*)0x4DCA20, &patch1, 1);
	}

	// Enable lag indicator.
	if (!GetBoolSetting("-lag"))
	{
		char byteArg = 0;
		WriteProcMem((char*)0x5D9368, &byteArg, 1);
	}

	// Prevent chat from being displayed ~ Motah
	if (GetBoolSetting("-nochat"))	
	{
		BYTE patch[] = { 0xC2, 0x10, 0x00 };
		WriteProcMem((char*)0x5A6250, &patch, 3);
	}

	// Maximum number of chat lines in chat history - M0tah
	BYTE byteArg = 0x7f;
	WriteProcMem((char*)0x4691D1, &byteArg, 1);

	// Maximum number of characters in chat input box - M0tah
	byteArg = 0x7f;
	WriteProcMem((char*)0x46A440, &byteArg, 1);

	// Double far plane of view frustum (nothing will be drawn beyond this) 
	{
		float patch = 250000.0f * 2;
		WriteProcMem((char*)0x610534, &patch, 4);
	}

	// Render asteroid fields from further away
	// Caution, if this is too big, collisions are turned off.
	{
		float patch = 1.74f * 2;
		WriteProcMem((char*)hCommon + 0x13DFDC, &patch, 4);
	}

	// Reduce screen shake when hull is hit
	{
		float patch = 0.005f;
		WriteProcMem((char*)hFreelancer + 0x11DB5C, &patch, 4);
	}

	// Seconds between updates of the distances in the contact list 
	{
		float patch = 1.0f;
		WriteProcMem((char*)0x5D7964, &patch, 4);
	}

	// Support rolling during mouse flight - adoxa
	{
		unsigned char patch[] = { 0xE8, 0x00, 0x00, 0x00, 0x00, 0x90 };
		uint *iAddr = (uint*)((char*)&patch + 1);
		*iAddr = (uint)HkCb_Roll - 0x4c7940 - 5;
		WriteProcMem((char*)0x4c7940, &patch, 6);

		DWORD dummy;
		VirtualProtect(((PDWORD)0x4c7f14), 4, PAGE_EXECUTE_READWRITE, &dummy );
	}

	// Support keyboard moves ship in turret view ~ adoxa
	{
		unsigned char patch[] = { 0x00 };
		WriteProcMem((char*)hFreelancer + 0xC7903, &patch, 1);

		unsigned char patch1[] = { 0xEB };
		WriteProcMem((char*)hFreelancer + 0xDBB12, &patch1, 1);
		WriteProcMem((char*)hFreelancer + 0xDBB58, &patch1, 1);
		WriteProcMem((char*)hFreelancer + 0xDBB9E, &patch1, 1);
		WriteProcMem((char*)hFreelancer + 0xDBBE4, &patch1, 1);
	}
	
	
	{
		// camera is independant of ship movement in turret view ~adoxa		
		unsigned char patch2[] = {
			0x9C, 0xA1, 0x44, 0x97, 0x67, 0x00, 0x83, 0xC0, 0xF8, 0x50, 0xFF,
			0x15, 0x70, 0x64, 0x5C, 0x00, 0x59, 0x91, 0x9D, 0x74, 0x0D, 0xA0,
			0xCA, 0xEC, 0x67, 0x00, 0x88, 0x81, 0xF9, 0x00, 0x00, 0x00, 0xEB,
			0x25, 0xB0, 0x00, 0x86, 0x81, 0xF9, 0x00, 0x00, 0x00, 0xA2, 0xCA,
			0xEC, 0x67, 0x00 };
		WriteProcMem((char*)hFreelancer + 0x14A65B, &patch2, 47);
	}

	{
		// camera is independant of ship movement in turret view PART 1 ~adoxa		
		unsigned char patch3[] = { 0xE8, 0x2F, 0x82, 0x0A, 0x00, 0x90 };
		WriteProcMem((char*)hFreelancer + 0x11D89C, &patch3, sizeof(patch3));
	}

	{
		// camera is independant of ship movement in turret view PART 2 ~adoxa 	
		unsigned char patch4[] = { 0xE8, 0xF0, 0x81, 0x0A, 0x00, 0x90 }; 
		WriteProcMem((char*)hFreelancer + 0x11D8BB, &patch4, sizeof(patch4));
	}

	{
		// camera is independant of ship movement in turret view PART 3 ~adoxa
		unsigned char patch5[] = { 0xD9, 0x15, 0x70, 0x52, 0x67, 0x00, 0xD8, 0x0D,
			0x84, 0x90, 0x67, 0x00, 0xD9, 0x1D, 0x7C, 0x90, 0x67, 0x00, 0xC3 };
		WriteProcMem((char*)hFreelancer + 0x1C5AB0, &patch5, sizeof(patch5));
		// fst flt_675270
		// label_0:
		// fisttp flt_679084 <-- camera rotation
		// fstp flt_67907c <-- camera rotation
		// retn
	}

	{
		// camera is independant of ship movement in turret view PART 4 ~adoxa
		unsigned char patch6[] = { 0xD9, 0x15, 0x7C, 0x52, 0x67, 0x00, 0xEB, 0xDE }; 
		WriteProcMem((char*)hFreelancer + 0x1C5AD0, &patch6, sizeof(patch6));
		// fst flt_67527c 
		// jmp label_0
	}

	// Support for turret zoom default tether distance
	{
		BYTE patch[] = { 0xB9, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE1, }; // mov ecx HkCb_InitShipNaked, jmp ecx
		DWORD *iAddr = (DWORD*)((char*)&patch + 1);
		*iAddr = reinterpret_cast<DWORD>((void*)HkCb_InitShipNaked);
		WriteProcMem((char*)0x54AEA5, &patch, 7);

		BYTE patch1[] = { 0xB9, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE1, }; // mov ecx HkCb_SetCamNaked, jmp ecx
		DWORD *iAddr1 = (DWORD*)((char*)&patch1 + 1);
		*iAddr1 = reinterpret_cast<DWORD>((void*)HkCb_SetCamNaked);
		WriteProcMem((char*)0x54A0D3, &patch1, 7);
	}

	// Patch get ids string
	{
		BYTE patch2[] = { 0xB9, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE1, 0x90 }; // mov ecx HkCb_GetIDSStringNaked, jmp ecx
		DWORD *iAddr = (DWORD*)((char*)&patch2 + 1);
		*iAddr = reinterpret_cast<DWORD>((void*)HkCb_GetIDSStringNaked);
		WriteProcMem((char*)0x4347E0, &patch2, 8);
	}

	// Patch get infocard
	{
		BYTE patch[] = { 0xB9, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE1 }; // mov ecx HkCb_GetInfocardNaked, jmp ecx
		DWORD *iAddr = (DWORD*)((char*)&patch + 1);
		*iAddr = reinterpret_cast<DWORD>((void*)HkCb_GetInfocardNaked);
		WriteProcMem((char*)0x057DA40, &patch, 7);
	} 

	// Patch keynames
	{
		static char patch0[] = "USER_ACTIVATE_JUMPDRIVE";
		*((char**)0x614FC4) = patch0;

		static char patch1[] = "USER_HUD"; 
		*((char**)0x614FC8) = patch1; // USER_MANEUVER_CRUISE

		static char patch2[] = "USER_TOGGLE_HUD_COLOR";
		*((char**)0x614FCC) = patch2; // USER_MANEUVER_TRADE_LANE

		static char patch3[] = "USER_LIGHTS";
		*((char**)0x614FD0) = patch3; // USER_MANEUVER_DRASTIC_EVADE

		static char patch4[] = "USER_PLAYER_INFO";
		*((char**)0x614FD8) = patch4; // USER_MANEUVER_STRAFE

		static char patch5[] = "USER_SELF_DESTRUCT";
		*((char**)0x614FDC) = patch5; // USER_MANEUVER_TRAIL_CLOSER

		static char patch6[] = "USER_SHIELDS";
		*((char**)0x614FE0) = patch6; // USER_MANEUVER_TRAIL_FARTHER

		static char patch7[] = "USER_GROUP_MARK_TARGET";
		*((char**)0x614FE4) = patch7; // USER_MANEUVER_CORKSCREW_EVADE

		static char patch8[] = "USER_GROUP_UNMARK_TARGET";
		*((char**)0x614FE8) = patch8; // USER_MANEUVER_SLIDE_EVADE

		BYTE patch[] = { 0xB9, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xE1 }; // mov ecx HkCb_HandleKeyCmdNaked, jmp ecx
		DWORD *iAddr = (DWORD*)((char*)&patch + 1);
		*iAddr = reinterpret_cast<DWORD>((void*)HkCb_HandleKeyCmdNaked);
		WriteProcMem((char*)0x576410, &patch, 7); 
	}

	// Allow any number of bots/bats to be displayed in the hud ~ adoxa
	{
		BYTE patch[] = { 0xEB };
		WriteProcMem((char*)hFreelancer + 0xDE314, patch, 1);
		WriteProcMem((char*)hFreelancer + 0xDE366, patch, 1);
	}

	// F3 docking works with FREIGHTER type ~ adoxa
	{
		BYTE patch1[] = { 0x62 };
		WriteProcMem((char*)hCommon + 0x63645, patch1, 1);
		BYTE patch2[] = { 0x02 };
		WriteProcMem((char*)hCommon + 0x63650, patch2, 1);
	}

	// Load infocards.ini
	LoadInfocards();

	// Allow a few problematic colors to be adjusted
	set_iColorHostile = rgb_to_bgr(GetHexSetting("-color-red="));
	if (!set_iColorHostile)
		set_iColorHostile = rgb_to_bgr(0xFF0000);

	set_iColorNeutral = rgb_to_bgr(GetHexSetting("-color-white="));
	if (!set_iColorNeutral)
		set_iColorNeutral = rgb_to_bgr(0xFFFFFF);

	set_iColorFriendly = rgb_to_bgr(GetHexSetting("-color-green="));
	if (!set_iColorFriendly)
		set_iColorFriendly = rgb_to_bgr(0x00FF00);

	set_iColorGroup = rgb_to_bgr(GetHexSetting("-color-pink="));
	if (!set_iColorGroup)
		set_iColorGroup = rgb_to_bgr(0x0000FF);

	/* // Support russian characters in chat
	{
		BYTE patch[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
		WriteProcMem((char*)0x1B01D0, patch, sizeof(patch));
	}

	{
		BYTE patch[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
		WriteProcMem((char*)0x1B01F0, patch, sizeof(patch));
	}

	{
		BYTE patch[] = { 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90, 0x90 };
		WriteProcMem((char*)0x1B0210, patch, sizeof(patch));
	}

	{
		BYTE patch[] = { 0x8B, 0x54, 0x24, 0x14, 0x50 };
		WriteProcMem((char*)0x1B0BC0, patch, sizeof(patch));
	} */

	// Patch to reduce planet spin speed ~ adoxa
	{
		BYTE patch[] = { 0xEB };
		WriteProcMem((char*)hCommon + 0x0E698E, patch, sizeof(patch));
	}

	// consider volume when buying external equipment ~adoxa
	byte patch1[] = { 0xEB };
	WriteProcMem((char*)hFreelancer + 0x838AF, patch1, sizeof(patch1));
}

/// Install command client and server side hooks and patches
void PublicServerClientPatchInstall()
{
	HMODULE hCommon = GetModuleHandleA("common.dll");

	// Patch to mount cloaking device on CM slot ~ Adoxa
	{
		byte patch[] = { 0x50, 0xDB, 0x26 };
		WriteProcMem((char*)hCommon + 0x139B74, patch, sizeof(patch));
	}

	// Fixes explosion damage at full strength ~ motah
	{
		byte patch[] = { 0x00 };
		WriteProcMem((char*)hCommon + 0x960C, patch, sizeof(patch));
	}
	
	{
		//common.dll 053048 0000->E31F = include external equipment in cargo size PART 1 ~adoxa		
		//common.dll 05330E 0000->E31F = include external equipment in cargo size PART 2 ~adoxa
		BYTE patch1[] = { 0xE3, 0x1F };
		WriteProcMem((char*)hCommon + 0x053048, patch1, sizeof(patch1));
		WriteProcMem((char*)hCommon + 0x05330E, patch1, sizeof(patch1));

		//common.dll 0A9BA3 18->00 = include external equipment in cargo size PART 3 ~adoxa
		//common.dll 0AA904 18->00 = include external equipment in cargo size PART 4 ~adoxa
		BYTE patch2[] = { 0x00 };
		WriteProcMem((char*)hCommon + 0x0A9BA3, patch2, sizeof(patch2));
		WriteProcMem((char*)hCommon + 0x0AA904, patch2, sizeof(patch2));
	}
}