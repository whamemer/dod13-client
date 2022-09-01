//========= Copyright (c) 1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#pragma once
#if !defined( DMC_BSPFILEH )
#define DMC_BSPFILEH

typedef struct lump_s
{
	int fileofs;
	int filelen;
} lump_t;

typedef struct dheader_s
{
    int version;
    lump_t lumps[15];
} dheader_t;

#endif // !DMC_BSPFILEH
