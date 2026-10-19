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
#pragma once
#if !defined(PM_MATERIALS_H)
#define PM_MATERIALS_H

#define CBTEXTURENAMEMAX		13 		// only load first n chars of name

// texture types of sound/materials.txt of Day of Defeat
#define CHAR_TEX_CONCRETE	'C'		// concrete is the default
#define CHAR_TEX_METAL		'M'
#define CHAR_TEX_GRATE		'G'		// grate/ventilation (V for ventilation was removed)
#define CHAR_TEX_DIRT		'D'
#define CHAR_TEX_TILE		'T'
#define CHAR_TEX_WOOD		'W'
#define CHAR_TEX_GRASS		'P'
#define CHAR_TEX_GLASS		'Y'
#define CHAR_TEX_SLOSH		'S'		// water
#define CHAR_TEX_ROCK		'R'
#define CHAR_TEX_SAND		'A'
#define CHAR_TEX_GRAVEL		'L'
#define CHAR_TEX_LEAVES		'E'		// leaves, bushes
#define CHAR_TEX_SNOW		'N'
#define CHAR_TEX_FLESH		'F'		// used in dod_anzio, the meat rack
#define CHAR_TEX_BRICK		'B'
#define CHAR_TEX_STUCCO		'Z'
#define CHAR_TEX_HEAVYMETAL	'H'		// dod_charlie, hedgehogs
#define CHAR_TEX_SKY		'K'
#endif//PM_MATERIALS_H
