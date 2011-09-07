//////////////////////////////////////////////////////////////////////
//	Project FLCoreSDK v1.1, modified for use in FLHook Plugin version
//--------------------------
//
//	File:			FLCoreDALib.h
//	Module:			FLCoreDALib.lib
//	Description:	Interface to DALib.dll
//
//	Web: www.skif.be/flcoresdk.php
//  
//
//////////////////////////////////////////////////////////////////////
#ifndef _FLCOREDALIB_H_
#define _FLCOREDALIB_H_

#include "FLCoreDefs.h"
#include <vector>

#pragma comment( lib, "FLCoreDALib.lib" )

class IMPORT CDPClient
{
public:
	 CDPClient(class CDPClient const &);
	 CDPClient(void);
	 virtual ~CDPClient(void);
	 class CDPClient & operator=(class CDPClient const &);
	 bool AddConnectAttempt(unsigned short const *,unsigned short *);
	 void CancelEnums(void);
	 void Cleanup(void);
	 void Close(void);
	 void DispatchMsgs(unsigned long);
	 bool EnumerateHost(unsigned short const *,unsigned long,unsigned long,unsigned long,unsigned long);
	 bool EnumerateHosts(void);
	 struct IDirectPlay8Client * GetClient(void);
	 long GetConnectResult(void);
	 static unsigned long __cdecl GetLastMsgTimestamp(void);
	 static unsigned long __cdecl GetLinkQuality(void);
	 static unsigned long __cdecl GetPingDelay(void);
	 static unsigned long __cdecl GetSendQueueSize(void);
	 static bool __cdecl IsPingOutstanding(void);
	 bool ProcessConnectAttempt(void);
	 bool Send(unsigned char *,unsigned long);
	 void SetGUID(struct _GUID &);
	 static void __cdecl SetSourcePort(unsigned long);

protected:
	 bool ConnectToServer(unsigned short const *,unsigned short const *);
	 static long __stdcall HandleClientMsg(void *,unsigned long,void *);
	 static unsigned long  m_dwLastMsgReceivedTime;
	 static unsigned long  m_dwLinkQuality;
	 static unsigned long  m_dwPingLastTime;
	 static unsigned long  m_dwPingReceiveCount;
	 static unsigned long  m_dwPingReceiveTime;
	 static unsigned long  m_dwPingSendCount;
	 static unsigned long  m_dwPingSendTime;
	 static unsigned long  m_dwRoundTripTime;
	 static unsigned long  m_dwSourcePort;

public:
	unsigned char data[OBJECT_DATA_SIZE];
};

class IMPORT CDPClientProxy
{
public:
	 CDPClientProxy(class CDPClientProxy const &);
	 CDPClientProxy(void);
	 virtual ~CDPClientProxy(void);
	 class CDPClientProxy & operator=(class CDPClientProxy const &);
	 bool Disconnect(void);
	 bool GetConnectionStats(struct _DPN_CONNECTION_INFO *);
	 double GetLinkSaturation(void);
	 unsigned int GetSendQBytes(void);
	 unsigned int GetSendQSize(void);
	 void OnMsgSent(unsigned long);
	 bool Send(void *,unsigned long);
	 bool SendMsgs(void);
	 void UpdateQueueHistory(void);

public:
	unsigned char data[OBJECT_DATA_SIZE];
};

class IMPORT CDPMessage
{
public:
	 CDPMessage(class CDPMessage const &);
	 CDPMessage(void);
	 virtual ~CDPMessage(void);
	 class CDPMessage & operator=(class CDPMessage const &);
	 unsigned char * GetData(void);
	 unsigned long const  GetSize(void);

public:
	unsigned char data[OBJECT_DATA_SIZE];
};

class IMPORT CDPMsgList
{
public:
	 CDPMsgList(class CDPMsgList const &);
	 CDPMsgList(void);
	 virtual ~CDPMsgList(void);
	 class CDPMsgList & operator=(class CDPMsgList const &);
	 void Add(class CDPMessage *);
	 void CopyList(class CDPMsgList &);
	 void ExtractMsgs(unsigned long,class CDPMsgList &);
	 class CDPMessage * GetNextMsg(void);
	 void Lock(void);
	 class CDPMessage * PeekNextMsg(void);
	 void SetEmptyEvent(void);
	 void Unlock(void);
	 void WaitForMsg(unsigned long);

public:
	unsigned char data[OBJECT_DATA_SIZE];
};

class IMPORT CDPServer
{
public:
	 CDPServer(class CDPServer const &);
	 CDPServer(void);
	 virtual ~CDPServer(void);
	 class CDPServer & operator=(class CDPServer const &);
	 bool BeginHosting(unsigned int);
	 static void __cdecl CrashCleanup(void);
	 virtual class CDPClientProxy * CreateClientProxy(void);
	 void DisconnectClient(unsigned long);
	 void DispatchMsgs(void);
	 bool GetConnectionStats(struct _DPN_CONNECTION_INFO *);
	 bool GetConnectionStats(class CDPClientProxy *,struct _DPN_CONNECTION_INFO *);
	 static unsigned long __cdecl GetLastMsgTimestamp(void);
	 void GetPort(std::vector<unsigned long> &);
	 unsigned int GetSendQBytes(class CDPClientProxy *);
	 unsigned int GetSendQSize(class CDPClientProxy *);
	 bool SendTo(class CDPClientProxy *,void *,unsigned long);
	 bool SetEnumResponse(void *,unsigned long);
	 void SetGUID(struct _GUID &);
	 void SetMaxPlayers(int);
	 void SetPassword(unsigned short const *);
	 void SetSessionName(unsigned short const *);
	 void StopHosting(void);

protected:
	 void GetHostAddresses(void);
	 static long __stdcall HandleDPServerMsg(void *,unsigned long,void *);
	 void ReleaseHostAddresses(void);
	 void UpdateDescription(void);
	 static unsigned long  m_dwLastMsgReceivedTime;
	 static class CDPServer *  m_pServer;

public:
	unsigned char data[OBJECT_DATA_SIZE];
};

struct IGunConnection
{
	enum ConnectStatus;
};

namespace Gun2
{
	struct GUNQueueMessage;
};

class IMPORT CGunWrapper
{
public:
	 CGunWrapper(class CGunWrapper const &);
	 CGunWrapper(char const *,struct _GUID &,char const *,unsigned long,unsigned short *);
	 virtual ~CGunWrapper(void);
	 class CGunWrapper & operator=(class CGunWrapper const &);
	 void DispatchQueue(void);
	 struct IGunBrowser * GetBrowser(void);
	 struct IGunConnection * GetConnection(void);
	 struct IGunHost * GetHost(void);
	 enum IGunConnection::ConnectStatus  GetStatus(void);
	 virtual long __stdcall Read(struct Gun2::GUNQueueMessage *);
	 static void __cdecl Shutdown(void);
	 void Update(void);

public:
	unsigned char data[OBJECT_DATA_SIZE];
};

struct IAlchemy;
struct IAnimation2;
struct IChannel;
struct ICOManager;
struct IFileSystem;
struct IDeformable;
struct IEngine2;
struct IEngine;
struct IFxEffectLibrary;
struct IFxRuntime;
struct IHardpoint;
struct IMatAnimLibrary;
struct IMaterialBatcher2;
struct IMaterialLibrary;
struct IRenderPipeline;
struct IRPDraw;
struct IRPIndexBuffer;
struct IRPVertexBuffer;
struct IRenderer2;
struct ISoundManager;
struct IStreamer2;
struct ITextureLibrary2;
struct IVMeshLibrary;
struct IVertexBufferManager;

namespace DALib
{
	IMPORT  struct IAlchemy *  Alchemy;
	IMPORT  struct IAnimation2 *  Anim;
	IMPORT  struct IChannel *  Channel;
	IMPORT  void __cdecl CloseData(void);
	IMPORT  struct ICOManager *  Dacom;
	IMPORT  struct IFileSystem *  Data;
	IMPORT  struct IDeformable *  Deform;
	IMPORT  struct IEngine2 *  Engine2;
	IMPORT  struct IEngine *  Engine;
	IMPORT  struct IFxEffectLibrary *  FxEffectLibrary;
	IMPORT  struct IFxRuntime *  FxRuntime;
	IMPORT  struct IHardpoint *  Hardpoint;
	IMPORT  struct IMatAnimLibrary *  MatAnimLibrary;
	IMPORT  struct IMaterialBatcher2 *  MaterialBatcher;
	IMPORT  struct IMaterialLibrary *  MaterialLib;
	IMPORT  void __cdecl OpenData(char const *);
	IMPORT  struct IRenderPipeline *  Pipeline;
	IMPORT  struct IRPDraw *  RPDraw;
	IMPORT  struct IRPIndexBuffer *  RPIndexBuffer;
	IMPORT  struct IRPVertexBuffer *  RPVertexBuffer;
	IMPORT  struct IRenderer2 *  Renderer;
	IMPORT  void __cdecl Shutdown(struct HWND__ *);
	IMPORT  struct ISoundManager *  Sound;
	IMPORT  bool __cdecl Startup(struct HWND__ *,char const *);
	IMPORT  struct IStreamer2 *  Streamer;
	IMPORT  struct ITextureLibrary2 *  TextureLib;
	IMPORT  struct IVMeshLibrary *  VMeshLibrary;
	IMPORT  struct IVertexBufferManager *  VertexBuffer;
};

class IMPORT IDPMsgHandler
{
public:
	 IDPMsgHandler(class IDPMsgHandler const &);
	 IDPMsgHandler(void);
	 class IDPMsgHandler & operator=(class IDPMsgHandler const &);
	 virtual void OnAddHost(struct _DPNMSG_ENUM_HOSTS_RESPONSE *);
	 virtual void OnConnect(long);
	 virtual void OnDisconnect(void);
	 virtual void OnReceive(unsigned char *,unsigned long);

public:
	unsigned char data[OBJECT_DATA_SIZE];
};

class IMPORT MD5Hash
{
public:
	 MD5Hash(class MD5Hash const &);
	 MD5Hash(void);
	 virtual ~MD5Hash(void);
	 class MD5Hash & operator=(class MD5Hash const &);
	 bool AddData(void * const,unsigned long);
	 char const * AsString(void);
	 bool CalcValue(void);
	 bool Compare(class MD5Hash &);
	 void FromString(char const *);
	 unsigned char * GetStatePtr(void);

protected:
	 void Decode(unsigned long *,unsigned char *,unsigned long);
	 void Encode(unsigned char *,unsigned long *,unsigned long);
	 void Transform(unsigned char *);

public:
	unsigned char data[OBJECT_DATA_SIZE];
};

IMPORT  bool (__cdecl* CriticalWarningFn)(unsigned int,char const *);
IMPORT  void (__cdecl* FatalErrorFn)(unsigned int,char const *);

#endif // _FLCOREDALIB_H_