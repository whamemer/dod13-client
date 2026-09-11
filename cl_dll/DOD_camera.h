//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#ifndef DOD_CAMERA_H
#define DOD_CAMERA_H

#ifdef _WIN32
#include "string.h"
#define strcasecmp _stricmp
#else
#include "strings.h"
#endif

#define SF_ENVMODEL_OFF 0x01
#define SF_ENVMODEL_ACTION_ON 0x10

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

dod_camera_t s_cameras[256];
int s_num_cameras;

#endif // DOD_CAMERA_H