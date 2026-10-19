/***
*
*	Copyright (c) 1996-2002, Valve LLC. All rights reserved.
*
*	This product contains software technology licensed from Id
*	Software, Inc. ("Id Technology").  Id Technology (c) 1996 Id Software, Inc.
*	All Rights Reserved.
*
*   Use, distribution, and modification of this source code and/or resulting
*   object code is restricted to non-commercial enhancements to products from
*   Valve LLC.  All other use, distribution, or modification is prohibited
*   without written permission from Valve LLC.
*
****/
//
// dod_camera.h
//
// cutscene cameras and the client side parsing of the entities of the map
//
#pragma once
#if !defined(DOD_CAMERA_H)
#define DOD_CAMERA_H

typedef enum
{
	DOD_CAMERA = 0,
	DOD_TARGET
} dod_cameratype_t;

typedef struct
{
	dod_cameratype_t type;
	char classname[32];
	char target[32];
	char targetname[32];
	int overlay;
	int fov;
	int subtitle;
	int fade;
} dod_camera_t;

struct key_value_pair_s
{
	char szKeyName[32];
	char szValue[128];
};

typedef struct
{
	char classname[32];
	key_value_pair_s kvPairs[128];
	int numpairs;
} dod_bspparse_ent;

void DoD_LoadClientEnts( const char *map );
dod_camera_t *DoD_FindTarget( const char *name );
void DoD_ReadCameraSettings( const char *Targetname );
void DoD_ScreenFades( int flags, float holdtime, int r, int g, int b, int a, int speed );

#endif // DOD_CAMERA_H
