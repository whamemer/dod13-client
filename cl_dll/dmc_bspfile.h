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
// dmc_bspfile.h
//
// the part of the BSP file format that the client reads itself (entity lump)
//
#pragma once
#if !defined(DMC_BSPFILE_H)
#define DMC_BSPFILE_H

typedef struct
{
	int fileofs;
	int filelen;
} lump_t;

typedef struct
{
	int version;
	lump_t lumps[15];
} dheader_t;

#endif // DMC_BSPFILE_H
