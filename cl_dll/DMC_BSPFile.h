//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#pragma once
#if !defined( DMC_BSPFILE_H )
#define DMC_BSPFILE_H

typedef struct
{
	int fileofs;
	int filelen;
} lump_t;

typedef struct
{
	int version;
	lump_t	lumps[15];
} dheader_t;

#endif // DMC_BSPFILE_H