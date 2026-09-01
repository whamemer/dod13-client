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
//  scope.cpp - implementation of the CHudScope class
//

#include "hud.h"
#include "dod_shared.h"

//extern engine_studio_api_t IEngineStudio;
extern vec3_t v_origin, v_angles;
extern float in_fov;
extern int g_iWeaponFlags;

//DECLARE_MESSAGE( m_Scope, Scope )

int CHudScope::Init( void )
{
	return 1;
}

int CHudScope::MsgFunc_Scope( const char *pszName, int iSize, void *pbuf )
{
	return 1;
}

void CHudScope::SetScope( int weaponId )
{

}

int CHudScope::VidInit( void )
{
	return 1;
}

void CHudScope::Reset( void )
{

}

int CHudScope::Draw( float flTime )
{
	return 1;
}

void CHudScope::Think( void )
{

}

void CHudScope::DrawTriApiScope( void )
{

}