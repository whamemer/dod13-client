//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once
#if !defined( DODCAMERA_H )
#define DODCAMERA_H

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
    struct key_value_pair_s kvPairs[128];
    int numpairs;
} dod_bspparse_ent;;

dod_camera_t s_cameras[256];
int s_num_cameras;
#endif // DODCAMERA_H
