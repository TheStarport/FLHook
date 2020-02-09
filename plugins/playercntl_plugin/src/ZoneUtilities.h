// Player Control plugin for FLHookPlugin
// Feb 2010 by Cannon
//
// This is free software; you can redistribute it and/or modify it as
// you wish without restriction. If you do then I would appreciate
// being notified and/or mentioned somewhere.

#ifndef __ZONE_UTILITIES_H__
#define __ZONE_UTILITIES_H__ 1

#include <windows.h>
#include <stdio.h>
#include <string>
#include <time.h>
#include <math.h>
#include <float.h>

struct SYSTEMINFO
{
	/** The system nickname */
	std::string sysNick;

	/** The system id */
	uint systemId;

	/** The system scale */
	float scale;
};

struct TransformMatrix
{
	float d[4][4];
};

struct ZONE
{
	/** The system nickname */
	std::string sysNick;

	/** The zone nickname */
	std::string zoneNick;

	/** The id of the system for this zone */
	uint systemId;

	/** The zone transformation matrix */
	TransformMatrix transform;

	/** The zone ellipsoid size */
	Vector size;

	/** The zone position */
	Vector pos;

	/** The damage this zone causes per second */
	int damage;

	/** Is this an encounter zone */
	bool encounter;
};

class JUMPPOINT
{
public:
	/** The system nickname */
	std::string sysNick;

	/** The jump point nickname */
	std::string jumpNick;

	/** The jump point destination system nickname */
	std::string jumpDestSysNick;

	/** The id of the system for this jump point. */
	uint System;

	/** The id of the jump point. */
	uint jumpID;

	/** The jump point destination system id */
	uint jumpDestSysID;
};

/** A map of system id to system info */
extern std::map<uint, SYSTEMINFO> mapSystems;

/** A map of system id to zones */
extern std::multimap<uint, ZONE> zones;
typedef std::multimap<uint, ZONE, std::less<uint> >::value_type zone_map_pair_t;
typedef std::multimap<uint, ZONE, std::less<uint> >::iterator zone_map_iter_t;

/** A map of system id to jumppoint info */
extern std::multimap<uint, JUMPPOINT> jumpPoints;
typedef std::multimap<uint, JUMPPOINT, std::less<uint> >::value_type jumppoint_map_pair_t;
typedef std::multimap<uint, JUMPPOINT, std::less<uint> >::iterator jumppoint_map_iter_t;

namespace ZoneUtilities
{
	void ReadUniverse();
	bool InZone(uint systemID, const Vector &pos, ZONE &rlz);
	bool InDeathZone(uint systemID, const Vector &pos, ZONE &rlz);
	SYSTEMINFO *GetSystemInfo(uint systemID);
}

#endif