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
//  hud_vgui2print.cpp - implementation of the CHudVGUI2Print class
//

#include "hud.h"
#include "dod_shared.h"

int CHudVGUI2Print::Init( void )
{
	return 1;
}

int CHudVGUI2Print::VidInit( void )
{
	return 1;
}

//vgui2::HFont CHudVGUI2Print::GetFont( void )

int CHudVGUI2Print::DrawVGUI2String( char *charMsg, int x, int y, float r, float g, float b )
{
	return 1;
}

int CHudVGUI2Print::DrawVGUI2String( wchar_t *msg, int x, int y, float r, float g, float b )
{
	return 1;
}

int CHudVGUI2Print::DrawVGUI2StringReverse( char *charMsg, int x, int y, float r, float g, float b )
{
	return 1;
}

int CHudVGUI2Print::DrawVGUI2StringReverse( wchar_t *msg, int x, int y, float r, float g, float b )
{
	return 1;
}

void CHudVGUI2Print::VGUI2HudPrintArgs( char *charMsg, char *sstr1, char *sstr2, char *sstr3, char *sstr4, int x, int y, float r, float g, float b )
{

}

void CHudVGUI2Print::VGUI2HudPrint( char *charMsg, int x, int y, float r, float g, float b )
{

}

int CHudVGUI2Print::Draw( float flTime )
{
	return 1;
}

int CHudVGUI2Print::GetHudFontHeight( void )
{
	return 1;
}

void CHudVGUI2Print::GetStringSize( const wchar_t *string, int *width, int *height )
{

}