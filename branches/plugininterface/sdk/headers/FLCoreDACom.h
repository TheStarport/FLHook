//////////////////////////////////////////////////////////////////////
//	Project FLCoreSDK v1.1
//--------------------------
//
//	File:			FLCoreDACom.h
//	Module:			FLCoreDACom.lib
//	Description:	Interface to dacom.dll
//
//	Web: www.skif.be/flcoresdk.php
//  
//
//////////////////////////////////////////////////////////////////////
#ifndef _FLCOREDACOM_H_
#define _FLCOREDACOM_H_

#include "FLCoreDefs.h"

#pragma comment( lib, "FLCoreDACom.lib" )

namespace DACOM_CRC
{
	IMPORT  int  CompareStringsI(char const *,char const *);
	IMPORT  unsigned long  GetCRC32(char const *,char const *);
	IMPORT  unsigned long  GetCRC32(char const *);
	IMPORT  unsigned long  GetContinuedCRC32(unsigned long,char);
	IMPORT  unsigned long  GetContinuedCRC32(unsigned long,char const *);
};

namespace LogStream
{
	IMPORT  void  FlushToDisk(void);
	IMPORT  void  LogEvent(char const *,float,unsigned long);
	IMPORT  void  LogNamedEvent(char const *,char const *,unsigned long);
	IMPORT  bool  Startup(char const *);
	IMPORT  void  Update(float);
};

IMPORT  void DACOM_Acquire();
IMPORT  void DACOM_GetDllVersion();
IMPORT  void DACOM_GetVersion();
IMPORT  void FDUMP();

#endif // _FLCOREDACOM_H_