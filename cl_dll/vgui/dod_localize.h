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
// dod_localize.h
//
// stand-in for vgui2::ILocalize: tokens come from resource/dod_english.txt, titles.txt is the fallback
//
#pragma once
#if !defined(DOD_LOCALIZE_H)
#define DOD_LOCALIZE_H

// loads the tokens of a VGUI2 localization file (UTF-16, KeyValues); returns the number of tokens that are known after it.
// The tokens of the files that are loaded later replace the ones of the same name, bAppend keeps the loaded tokens
int DoD_LoadLocalization( const char *pszFileName, bool bAppend = false );

// finds a token ("#Token" or "Token") in the loaded file, then in titles.txt; NULL when it is unknown
const char *DoD_FindLocalized( const char *pszToken );

// replaces %s1..%s4 of the format with the arguments
void DoD_ConstructLocalized( char *pszDest, int iDestSize, const char *pszFormat, const char *pszArg1, const char *pszArg2, const char *pszArg3, const char *pszArg4 );

#endif // DOD_LOCALIZE_H
