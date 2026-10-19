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

#include "extdll.h"
#include "util.h"
#include "cbase.h"
#include "weapons.h"

#include "dod_shared.h"

LINK_ENTITY_TO_CLASS( weapon_gerknife, CGerKnife )

extern p_wpninfo_s P_WpnInfo[];

void CGerKnife::Spawn( void )
{
	CMeleeWeapon::Spawn( WEAPON_GERKNIFE );
}

void CGerKnife::Precache( void )
{
	PRECACHE_MODEL( P_WpnInfo[WEAPON_GERKNIFE].vmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_GERKNIFE].wmodel );
	PRECACHE_MODEL( P_WpnInfo[WEAPON_GERPARAKNIFE].vmodel );

	CMeleeWeapon::Precache();
}

int CGerKnife::GetItemInfo( ItemInfo *p )
{
	p->pszName = STRING( pev->classname );
	p->pszAmmo1 = NULL;
	p->iMaxAmmo1 = -1;
	p->pszAmmo2 = NULL;
	p->iMaxAmmo2 = -1;
	p->iMaxClip = WEAPON_NOCLIP;
	p->iSlot = 0;
	p->iPosition = 1;
	p->iId = WEAPON_GERKNIFE;
	p->iWeight = 0;

	return 1;
}

int CGerKnife::GetSlashAnim( int m_iSwing )
{
	int i_Anim = KNIFE_SLASH1;

	if( m_iSwing % 2 == 1 )
		i_Anim = KNIFE_SLASH2;

	return i_Anim;
}

int CGerKnife::GetDrawAnim( void )
{
	return KNIFE_DRAW;
}

int CGerKnife::GetIdleAnim( void )
{
	return -1;
}
