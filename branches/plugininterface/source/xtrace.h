/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * xtrace.h by Hector J. Rivas, torjo2k@hotmail.com from "ExtendedTrace"
 * by Zoltan Csizmadia, zoltan_csizmadia@yahoo.com
 * 
 * A Win32 VC++ 6.0 implementation of the __FUNCTION__ macro that works for me.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef _XTRACE_H_
#define _XTRACE_H_

#if defined(_DEBUG) && defined(WIN32)

#include <windows.h>
#include <tchar.h>

#pragma comment(lib, "imagehlp.lib")
#pragma warning(disable : 4172)			// returning address of a temp

#define BUFFERSIZE 512

BOOL	InitSymInfo(PCSTR lpszInitialSymbolPath = NULL, BOOL bSysPath = FALSE);
void	InitSymPath(PSTR lpszSymbolPath, PCSTR lpszIniPath, BOOL bSysPath);
BOOL	GetFuncInfo(ULONG fnAddress, ULONG stackAddress, LPTSTR lpszSymbol);
LPCTSTR GetFuncName();
BOOL	KillSymInfo();

#define __FUNCTION__ GetFuncName()

#else
#include <windows.h>
#include <tchar.h>


BOOL InitSymInfo(PCSTR) ;
BOOL KillSymInfo() ;

#define __FUNCTION__ _T("")

#endif

#endif